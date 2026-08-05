#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <utf/string.h>

#include "../include/url/idna.h"

/**
 * Runs the domain parser against the test cases for verifying UTS #46
 * conformance, of which only those of nontransitional ToASCII apply.
 *
 * https://www.unicode.org/Public/idna/16.0.0/IdnaTestV2.txt
 */

// The status codes of the flags that the domain parser sets to false, being
// CheckHyphens, UseSTD3ASCIIRules, and VerifyDnsLength. A test case recording
// no other status code is expected to succeed.
static const char *ignored[] = {"V2", "V3", "U1", "A4_1", "A4_2", "X4_2"};

static void
trim (char **start, char *end) {
  while (*start < end && (**start == ' ' || **start == '\t')) (*start)++;

  while (end > *start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n' || end[-1] == '\r')) {
    *--end = '\0';
  }
}

static void
append_code_point (utf8_string_t *result, utf32_t c) {
  int err;

  if (c < 0x80) {
    err = utf8_string_append_character(result, (utf8_t) c);
    assert(err == 0);

    return;
  }

  utf8_t bytes[4];

  size_t len;

  if (c < 0x800) {
    len = 2;
    bytes[0] = (utf8_t) (0xc0 | (c >> 6));
  } else if (c < 0x10000) {
    len = 3;
    bytes[0] = (utf8_t) (0xe0 | (c >> 12));
  } else {
    len = 4;
    bytes[0] = (utf8_t) (0xf0 | (c >> 18));
  }

  for (size_t i = 1; i < len; i++) {
    bytes[i] = (utf8_t) (0x80 | ((c >> (6 * (len - 1 - i))) & 0x3f));
  }

  err = utf8_string_append_literal(result, bytes, len);
  assert(err == 0);
}

// Reads the four hex digits of a \uXXXX escape, which must not be read past as
// the escape may well be followed by another hex digit.
static utf32_t
hex (const char *input) {
  utf32_t c = 0;

  for (size_t i = 0; i < 4; i++) {
    char digit = input[i];

    if (digit >= '0' && digit <= '9') c = c * 16 + (utf32_t) (digit - '0');
    else if (digit >= 'a' && digit <= 'f') c = c * 16 + (utf32_t) (digit - 'a' + 10);
    else if (digit >= 'A' && digit <= 'F') c = c * 16 + (utf32_t) (digit - 'A' + 10);
    else assert(false);
  }

  return c;
}

// Resolves the escapes of a column, returning false if it holds an unpaired
// surrogate, which no domain can be made to contain.
static bool
unescape (const char *input, utf8_string_t *result) {
  // A column of "" is an explicitly empty value.
  if (strcmp(input, "\"\"") == 0) return true;

  for (const char *p = input; *p != '\0';) {
    utf32_t c;

    if (p[0] == '\\' && p[1] == 'u') {
      c = hex(p + 2);
      p += 6;
    } else if (p[0] == '\\' && p[1] == 'x' && p[2] == '{') {
      char *end;

      c = (utf32_t) strtoul(p + 3, &end, 16);
      p = *end == '}' ? end + 1 : end;
    } else {
      // Anything else is already UTF-8 encoded.
      int err = utf8_string_append_character(result, (utf8_t) *p++);
      assert(err == 0);

      continue;
    }

    if (c >= 0xd800 && c <= 0xdbff) {
      // A supplementary code point may be escaped as a surrogate pair.
      if (p[0] != '\\' || p[1] != 'u') return false;

      utf32_t low = hex(p + 2);

      if (low < 0xdc00 || low > 0xdfff) return false;

      c = 0x10000 + ((c - 0xd800) << 10) + (low - 0xdc00);

      p += 6;
    } else if (c >= 0xdc00 && c <= 0xdfff) {
      return false;
    }

    append_code_point(result, c);
  }

  return true;
}

// A blank column takes the value of the column it defers to.
static const char *
inherit (const char *value, const char *fallback) {
  return value[0] == '\0' ? fallback : value;
}

// Tells whether a column of status codes holds any code that the domain parser
// is expected to record.
static bool
has_status (const char *status) {
  for (const char *p = status; *p != '\0';) {
    if (*p == '[' || *p == ']' || *p == ',' || *p == ' ') {
      p++;
      continue;
    }

    const char *start = p;

    while (*p != '\0' && *p != ']' && *p != ',' && *p != ' ') p++;

    size_t len = (size_t) (p - start);

    bool skip = false;

    for (size_t i = 0; i < sizeof(ignored) / sizeof(ignored[0]); i++) {
      if (strlen(ignored[i]) == len && strncmp(ignored[i], start, len) == 0) {
        skip = true;
        break;
      }
    }

    if (!skip) return true;
  }

  return false;
}

int
main (int argc, char *argv[]) {
  assert(argc == 2);

  FILE *file = fopen(argv[1], "r");
  assert(file != NULL);

  char line[4096];

  size_t tested = 0, skipped = 0, failed = 0;

  while (fgets(line, sizeof(line), file) != NULL) {
    // A comment runs to the end of the line, and no column holds a hash mark.
    char *comment = strchr(line, '#');

    if (comment != NULL) *comment = '\0';

    char *columns[7];

    size_t n = 0;

    for (char *column = line; n < 7;) {
      char *separator = strchr(column, ';');

      columns[n++] = column;

      if (separator == NULL) break;

      *separator = '\0';

      column = separator + 1;
    }

    // Anything with fewer columns is not a test case.
    if (n != 7) continue;

    for (size_t i = 0; i < n; i++) {
      trim(&columns[i], columns[i] + strlen(columns[i]));
    }

    const char *source = columns[0];
    const char *to_unicode = inherit(columns[1], source);
    const char *to_ascii = inherit(columns[3], to_unicode);
    const char *status = inherit(columns[4], columns[2]);

    utf8_string_t input, expected, actual;

    utf8_string_init(&input);
    utf8_string_init(&expected);
    utf8_string_init(&actual);

    if (unescape(source, &input) && unescape(to_ascii, &expected)) {
      int err = url__idna_to_ascii(utf8_string_view(&input), &actual);

      bool ok;

      if (has_status(status)) {
        ok = err != 0;
      } else {
        ok = err == 0 && utf8_string_compare(&actual, &expected) == 0;
      }

      // Converting a domain is idempotent, so converting the result again must
      // leave it unchanged.
      if (err == 0) {
        utf8_string_t again;
        utf8_string_init(&again);

        assert(url__idna_to_ascii(utf8_string_view(&actual), &again) == 0);
        assert(utf8_string_compare(&again, &actual) == 0);

        utf8_string_destroy(&again);
      }

      if (!ok) {
        failed++;

        printf(
          "  %s: expected %s%.*s%s, got %s%.*s%s\n",
          source,
          has_status(status) ? "failure (" : "",
          (int) expected.len,
          expected.data,
          has_status(status) ? ")" : "",
          err != 0 ? "failure (" : "",
          (int) actual.len,
          actual.data,
          err != 0 ? ")" : ""
        );
      }

      tested++;
    } else {
      skipped++;
    }

    utf8_string_destroy(&input);
    utf8_string_destroy(&expected);
    utf8_string_destroy(&actual);
  }

  fclose(file);

  printf("%zu passed, %zu failed, %zu skipped\n", tested - failed, failed, skipped);

  assert(tested > 0);
  assert(failed == 0);
}
