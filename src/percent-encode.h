#ifndef URL_PERCENT_ENCODE
#define URL_PERCENT_ENCODE

#include <utf.h>
#include <utf/string.h>

#include "infra.h"
#include "percent-encode-set.h"

// https://url.spec.whatwg.org/#percent-encode
static const utf8_t url__percent_encoded[768] =
  "%00%01%02%03%04%05%06%07"
  "%08%09%0A%0B%0C%0D%0E%0F"
  "%10%11%12%13%14%15%16%17"
  "%18%19%1A%1B%1C%1D%1E%1F"
  "%20%21%22%23%24%25%26%27"
  "%28%29%2A%2B%2C%2D%2E%2F"
  "%30%31%32%33%34%35%36%37"
  "%38%39%3A%3B%3C%3D%3E%3F"
  "%40%41%42%43%44%45%46%47"
  "%48%49%4A%4B%4C%4D%4E%4F"
  "%50%51%52%53%54%55%56%57"
  "%58%59%5A%5B%5C%5D%5E%5F"
  "%60%61%62%63%64%65%66%67"
  "%68%69%6A%6B%6C%6D%6E%6F"
  "%70%71%72%73%74%75%76%77"
  "%78%79%7A%7B%7C%7D%7E%7F"
  "%80%81%82%83%84%85%86%87"
  "%88%89%8A%8B%8C%8D%8E%8F"
  "%90%91%92%93%94%95%96%97"
  "%98%99%9A%9B%9C%9D%9E%9F"
  "%A0%A1%A2%A3%A4%A5%A6%A7"
  "%A8%A9%AA%AB%AC%AD%AE%AF"
  "%B0%B1%B2%B3%B4%B5%B6%B7"
  "%B8%B9%BA%BB%BC%BD%BE%BF"
  "%C0%C1%C2%C3%C4%C5%C6%C7"
  "%C8%C9%CA%CB%CC%CD%CE%CF"
  "%D0%D1%D2%D3%D4%D5%D6%D7"
  "%D8%D9%DA%DB%DC%DD%DE%DF"
  "%E0%E1%E2%E3%E4%E5%E6%E7"
  "%E8%E9%EA%EB%EC%ED%EE%EF"
  "%F0%F1%F2%F3%F4%F5%F6%F7"
  "%F8%F9%FA%FB%FC%FD%FE%FF";

// https://url.spec.whatwg.org/#percent-decode
static const uint8_t url__percent_decoded[256] = {
  ['0'] = 0,
  ['1'] = 1,
  ['2'] = 2,
  ['3'] = 3,
  ['4'] = 4,
  ['5'] = 5,
  ['6'] = 6,
  ['7'] = 7,
  ['8'] = 8,
  ['9'] = 9,
  ['A'] = 10,
  ['a'] = 10,
  ['B'] = 11,
  ['b'] = 11,
  ['C'] = 12,
  ['c'] = 12,
  ['D'] = 13,
  ['d'] = 13,
  ['E'] = 14,
  ['e'] = 14,
  ['F'] = 15,
  ['f'] = 15,
};

static inline int
url__percent_encode_character (utf8_t character, url_percent_encode_set_t percent_encode_set, utf8_string_t *result) {
  int err;

  if (url__is_in_percent_encode_set(percent_encode_set, character)) {
    err = utf8_string_append_literal(result, &url__percent_encoded[(character << 1) + character], 3);
  } else {
    err = utf8_string_append_character(result, character);
  }

  return err;
}

static inline int
url__percent_encode_string (const utf8_string_view_t view, url_percent_encode_set_t percent_encode_set, utf8_string_t *result) {
  int err;

  size_t i = 0;

  for (size_t n = view.len; i < n; i++) {
    if (url__is_in_percent_encode_set(percent_encode_set, view.data[i])) {
      break;
    }
  }

  if (i == view.len) return utf8_string_append_view(result, view);

  err = utf8_string_reserve(result, result->len + view.len);
  if (err < 0) return err;

  err = utf8_string_append_view(result, utf8_string_view_substring(view, 0, i));
  if (err < 0) return err;

  for (size_t n = view.len; i < n; i++) {
    err = url__percent_encode_character(view.data[i], percent_encode_set, result);
    if (err < 0) return err;
  }

  return 0;
}

static inline int
url__percent_decode_character (const utf8_t *character, utf8_string_t *result) {
  return utf8_string_append_character(result, url__percent_decoded[character[0]] * 0x10 + url__percent_decoded[character[1]]);
}

static inline int
url__percent_decode_string (const utf8_string_view_t view, utf8_string_t *result) {
  int err;

  size_t i = 0;

  for (size_t n = view.len; i < n; i++) {
    if (view.data[i] == 0x25) {
      break;
    }
  }

  err = utf8_string_reserve(result, result->len + view.len);
  if (err < 0) return err;

  err = utf8_string_append_view(result, utf8_string_view_substring(view, 0, i));
  if (err < 0) return err;

  for (size_t n = view.len; i < n; i++) {
    utf8_t c = view.data[i];

    if (
      c == 0x25 &&
      i < n - 2 &&
      url__is_ascii_alphanumeric(view.data[i + 1]) &&
      url__is_ascii_alphanumeric(view.data[i + 2])
    ) {
      err = url__percent_decode_character(&view.data[i + 1], result);
      if (err < 0) return err;

      i += 2;
    } else {
      err = utf8_string_append_character(result, c);
      if (err < 0) return err;
    }
  }

  return 0;
}

#endif // URL_PERCENT_ENCODE
