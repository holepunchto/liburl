#ifndef URL_PARSE_H
#define URL_PARSE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <utf.h>
#include <utf/string.h>

#include "../include/url.h"
#include "character-set.h"
#include "infra.h"
#include "percent-encode.h"
#include "serialize.h"
#include "type.h"

typedef enum : uint8_t {
  url_state_scheme_start = 1,
  url_state_scheme,
  url_state_no_scheme,
  url_state_special_relative_or_authority,
  url_state_path_or_authority,
  url_state_relative,
  url_state_relative_slash,
  url_state_special_authority_slashes,
  url_state_special_authority_ignore_slashes,
  url_state_authority,
  url_state_host,
  url_state_hostname,
  url_state_port,
  url_state_file,
  url_state_file_slash,
  url_state_file_host,
  url_state_path_start,
  url_state_path,
  url_state_opaque_path,
  url_state_query,
  url_state_fragment,
} url_state_t;

// https://url.spec.whatwg.org/#forbidden-host-code-point
static url_character_set_t url__forbidden_host_character_set = {
  // 00    01     02     03     04     05     06     07
  0x01 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 08    09     0a     0b     0c     0d     0e     0f
  0x00 | 0x02 | 0x04 | 0x00 | 0x00 | 0x20 | 0x00 | 0x00,
  // 10    11     12     13     14     15     16     17
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 18    19     1a     1b     1c     1d     1e     1f
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 20    21     22     23     24     25     26     27
  0x01 | 0x00 | 0x00 | 0x08 | 0x00 | 0x00 | 0x00 | 0x00,
  // 28    29     2a     2b     2c     2d     2e     2f
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x80,
  // 30    31     32     33     34     35     36     37
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 38    39     3a     3b     3c     3d     3e     3f
  0x00 | 0x00 | 0x04 | 0x00 | 0x10 | 0x00 | 0x40 | 0x80,
  // 40    41     42     43     44     45     46     47
  0x01 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 48    49     4a     4b     4c     4d     4e     4f
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 50    51     52     53     54     55     56     57
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 58    59     5a     5b     5c     5d     5e     5f
  0x00 | 0x00 | 0x00 | 0x08 | 0x10 | 0x20 | 0x40 | 0x00,
  // 60    61     62     63     64     65     66     67
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 68    69     6a     6b     6c     6d     6e     6f
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 70    71     72     73     74     75     76     77
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 78    79     7a     7b     7c
  0x00 | 0x00 | 0x00 | 0x00 | 0x10,
};

// https://url.spec.whatwg.org/#forbidden-domain-code-point
static url_character_set_t url__forbidden_domain_character_set = {
  // 00    01     02     03     04     05     06     07
  0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
  // 08    09     0a     0b     0c     0d     0e     0f
  0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
  // 10    11     12     13     14     15     16     17
  0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
  // 18    19     1a     1b     1c     1d     1e     1f
  0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
  // 20    21     22     23     24     25     26     27
  0x01 | 0x00 | 0x00 | 0x08 | 0x00 | 0x20 | 0x00 | 0x00,
  // 28    29     2a     2b     2c     2d     2e     2f
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x80,
  // 30    31     32     33     34     35     36     37
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 38    39     3a     3b     3c     3d     3e     3f
  0x00 | 0x00 | 0x04 | 0x00 | 0x10 | 0x00 | 0x40 | 0x80,
  // 40    41     42     43     44     45     46     47
  0x01 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 48    49     4a     4b     4c     4d     4e     4f
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 50    51     52     53     54     55     56     57
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 58    59     5a     5b     5c     5d     5e     5f
  0x00 | 0x00 | 0x00 | 0x08 | 0x10 | 0x20 | 0x40 | 0x00,
  // 60    61     62     63     64     65     66     67
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 68    69     6a     6b     6c     6d     6e     6f
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 70    71     72     73     74     75     76     77
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 78    79     7a     7b     7c
  0x00 | 0x00 | 0x00 | 0x00 | 0x10,
};

// https://url.spec.whatwg.org/#windows-drive-letter
static inline bool
url__is_windows_drive_letter (const utf8_string_view_t input) {
  return input.len == 2 && url__is_ascii_alpha(input.data[0]) && (input.data[1] == 0x3a || input.data[1] == 0x7c);
}

// https://url.spec.whatwg.org/#normalized-windows-drive-letter
static inline bool
url__is_normalized_windows_drive_letter (const utf8_string_view_t input) {
  return input.len == 2 && url__is_ascii_alpha(input.data[0]) && input.data[1] == 0x3a;
}

// https://url.spec.whatwg.org/#start-with-a-windows-drive-letter
static inline bool
url__starts_with_windows_drive_letter (const utf8_string_view_t input) {
  return (
    input.len >= 2 &&
    url__is_normalized_windows_drive_letter(utf8_string_view_substring(input, 0, 2)) &&
    (input.len == 2 || input.data[2] == 0x2f || input.data[2] == 0x5c || input.data[2] == 0x3f || input.data[2] == 0x23)
  );
}

// https://url.spec.whatwg.org/#shorten-a-urls-path
static inline void
url__shorten_path (url_t *url) {
  assert((url->flags & url_has_opaque_path) == 0);

  utf8_string_view_t path = url_get_path(url);

  size_t i = utf8_string_view_last_index_of_character(path, path.len - 1, '/');

  if (i == (size_t) -1) return;

  if (
    url->type == url_type_file &&
    i == 0 &&
    url__is_normalized_windows_drive_letter(utf8_string_view_substring(path, 1, path.len))
  ) {
    return;
  }

  url->href.len = url->components.path_start + i;
}

// https://url.spec.whatwg.org/#single-dot-path-segment
static inline bool
url__is_single_dot_path_segment (const utf8_string_view_t path) {
  size_t len = path.len;
  const utf8_t *data = path.data;

  // .
  if (len == 1) return data[0] == '.';

  // %2e
  return len == 3 && data[0] == '%' && data[1] == '2' && url__to_ascii_lowercase(data[2]) == 'e';
}

// https://url.spec.whatwg.org/#double-dot-path-segment
static inline bool
url__is_double_dot_path_segment (const utf8_string_view_t path) {
  size_t len = path.len;
  const utf8_t *data = path.data;

  // ..
  if (len == 2 && data[0] == '.' && data[0] == '.') {
    return true;
  }

  if (len == 4) {
    // .%2e
    if (data[0] == '.' && data[1] == '%' && data[2] == '2' && url__to_ascii_lowercase(data[3]) == 'e') {
      return true;
    }

    // %2e.
    if (data[0] == '%' && data[1] == '2' && url__to_ascii_lowercase(data[2]) == 'e' && data[3] == '.') {
      return true;
    }
  }

  // %2e%2e
  return (
    len == 6 &&
    data[0] == '%' && data[1] == '2' && url__to_ascii_lowercase(data[2]) == 'e' &&
    data[3] == '%' && data[4] == '2' && url__to_ascii_lowercase(data[5]) == 'e'
  );
}

// https://url.spec.whatwg.org/#ends-in-a-number-checker
static inline bool
url__ends_in_a_number (const utf8_string_view_t input) {
  size_t i = utf8_string_view_last_index_of_character(input, input.len - 1, '.');

  utf8_string_view_t last = utf8_string_view_substring(input, i + 1, input.len);

  if (utf8_string_view_empty(last)) {
    if (i + 1 == 0) return false;

    size_t j = utf8_string_view_last_index_of_character(input, i - 1, '.');

    last = utf8_string_view_substring(input, j + 1, i);

    if (utf8_string_view_empty(last)) return false;
  }

  if (last.len > 1 && last.data[0] == 0x30 && url__to_ascii_lowercase(last.data[1]) == 0x78) {
    last = utf8_string_view_substring(last, 2, last.len);

    for (size_t i = 0; i < last.len; i++) {
      if (!url__is_ascii_hex_digit(last.data[i])) return false;
    }
  } else {
    for (size_t i = 0; i < last.len; i++) {
      if (!url__is_ascii_digit(last.data[i])) return false;
    }
  }

  return true;
}

// https://url.spec.whatwg.org/#ipv4-number-parser
static inline uint64_t
url__parse_ipv4_number (utf8_string_view_t input) {
  if (utf8_string_view_empty(input)) goto err;

  uint8_t r = 10;

  if (input.len > 1 && input.data[0] == 0x30 && url__to_ascii_lowercase(input.data[1]) == 0x78) {
    input = utf8_string_view_substring(input, 2, input.len);

    if (utf8_string_view_empty(input)) goto err;

    r = 16;
  } else if (input.len > 1 && input.data[0] == 0x30) {
    input = utf8_string_view_substring(input, 1, input.len);

    if (utf8_string_view_empty(input)) goto err;

    r = 8;
  }

  uint64_t result = 0;

  for (size_t i = 0; i < input.len; i++) {
    utf8_t c = input.data[i];

    uint8_t value;

    if (url__is_ascii_digit(c)) {
      value = c - 0x30;

      if (value >= r) goto err;
    } else if (r == 16 && url__is_ascii_upper_hex_digit(c)) {
      value = c - 0x41 + 10;
    } else if (r == 16 && url__is_ascii_lower_hex_digit(c)) {
      value = c - 0x61 + 10;
    } else {
      goto err;
    }

    result = result * r + value;
  }

  return result;

err:
  return (uint64_t) -1;
}

// https://url.spec.whatwg.org/#concept-ipv4-parser
static inline int
url__parse_ipv4 (utf8_string_view_t input, utf8_string_t *result) {
  uint32_t address = 0;

  uint8_t parts = 0;

  while (!utf8_string_view_empty(input)) {
    size_t i = utf8_string_view_index_of_character(input, 0, '.');

    utf8_string_view_t part = utf8_string_view_substring(input, 0, i);

    input = utf8_string_view_substring(input, i + 1, input.len);

    uint64_t value = url__parse_ipv4_number(part);

    if (value == (uint64_t) -1) goto err;

    if (i == (size_t) -1) {
      size_t bits = 32 - parts * 8;

      if (value > (1 << bits)) goto err;

      address = (address << bits) | value;

      break;
    }

    if (value > 255 || parts == 3) goto err;

    address = (address << 8) | value;

    parts++;
  }

  return url__serialize_ipv4(address, result);

err:
  return -1;
}

// https://url.spec.whatwg.org/#concept-ipv6-parser
static inline int
url__parse_ipv6 (utf8_string_view_t input, utf8_string_t *result) {
  int err;

  if (utf8_string_view_empty(input)) goto err;

  uint16_t address[8] = {0};

  uint8_t piece_index = 0;
  uint8_t compress = (uint8_t) -1;

  const utf8_t *pointer = input.data;
  const utf8_t *eof = &input.data[input.len];

  if (input.data[0] == 0x3a) {
    if (input.len == 1 || input.data[1] != 0x3a) goto err;

    pointer += 2;
    compress = ++piece_index;
  }

  while (pointer != eof) {
    if (piece_index == 8) goto err;

    if (*pointer == 0x3a) {
      if (compress != (uint8_t) -1) goto err;

      pointer++;
      compress = ++piece_index;
    }

    uint16_t value = 0, length = 0;

    while (length < 4 && pointer != eof && url__is_ascii_hex_digit(*pointer)) {
      value = value * 0x10 + url__hex_decoded[*pointer];

      pointer++;
      length++;
    }

    if (pointer != eof && *pointer == 0x2e) {
      if (length == 0) goto err;

      pointer -= length;

      if (piece_index > 6) goto err;

      uint16_t numbers_seen = 0;

      while (pointer != eof) {
        uint16_t ipv4_piece = (uint16_t) -1;

        if (numbers_seen > 0) {
          if (*pointer == 0x2e && numbers_seen < 4) {
            pointer++;
          } else {
            goto err;
          }
        }

        if (pointer == eof || !url__is_ascii_digit(*pointer)) {
          goto err;
        }

        while (pointer != eof && url__is_ascii_digit(*pointer)) {
          uint8_t number = *pointer - 0x30;

          if (ipv4_piece == (uint16_t) -1) {
            ipv4_piece = number;
          } else if (ipv4_piece == 0) {
            goto err;
          } else {
            ipv4_piece = ipv4_piece * 10 + number;
          }

          if (ipv4_piece > 255) goto err;

          pointer++;
        }

        address[piece_index] = address[piece_index] * 0x100 + ipv4_piece;

        numbers_seen++;

        if (numbers_seen == 2 || numbers_seen == 4) piece_index++;
      }

      if (numbers_seen != 4) goto err;

      break;
    } else if (*pointer == 0x3a) {
      pointer++;

      if (pointer == eof) goto err;
    } else if (pointer != eof) {
      goto err;
    }

    address[piece_index] = value;

    piece_index++;
  }

  if (compress != (uint8_t) -1) {
    int8_t swaps = piece_index - compress;

    piece_index = 7;

    while (piece_index != 0 && swaps > 0) {
      uint16_t tmp = address[piece_index];

      address[piece_index] = address[compress + swaps - 1];
      address[compress + swaps - 1] = tmp;

      piece_index--;
      swaps--;
    }
  } else if (piece_index != 8) {
    goto err;
  }

  err = utf8_string_append_character(result, '[');
  if (err < 0) goto err;

  err = url__serialize_ipv6(address, result);
  if (err < 0) goto err;

  err = utf8_string_append_character(result, ']');
  if (err < 0) goto err;

  return 0;

err:
  return -1;
}

// https://url.spec.whatwg.org/#concept-opaque-host-parser
static inline int
url__parse_opaque_host (const utf8_string_view_t input, utf8_string_t *result) {
  if (url__contains_from_character_set(url__forbidden_host_character_set, input)) {
    return -1;
  }

  return url__percent_encode_string(input, url__c0_control_percent_encode_set, result);
}

// https://url.spec.whatwg.org/#concept-host-parser
static inline int
url__parse_host (const utf8_string_view_t input, bool is_opaque, utf8_string_t *result) {
  int err;

  if (input.len > 0 && input.data[0] == 0x5b) {
    if (input.data[input.len - 1] != 0x5d) return -1;

    return url__parse_ipv6(utf8_string_view_substring(input, 1, input.len - 1), result);
  }

  if (is_opaque) return url__parse_opaque_host(input, result);

  assert(input.len != 0);

  if (!url__contains_from_character_set(url__forbidden_domain_character_set, input)) {
    err = utf8_string_append_view(result, input);
    if (err < 0) return err;

    return 0;
  }

  utf8_string_t domain;
  utf8_string_init(&domain);

  err = url__percent_decode_string(input, &domain);
  if (err < 0) goto err;

  // TODO Domain to ASCII

  utf8_string_t ascii_domain = domain;

  if (url__ends_in_a_number(utf8_string_view(&ascii_domain))) {
    err = url__parse_ipv4(utf8_string_view(&ascii_domain), result);
    if (err < 0) goto err;
  } else {
    err = utf8_string_append(result, &ascii_domain);
    if (err < 0) goto err;
  }

  utf8_string_destroy(&domain);

  return 0;

err:
  utf8_string_destroy(&domain);

  return -1;
}

static inline int
url__parse (url_t *url, const utf8_string_view_t input, const url_t *base) {
  int err;

  url_state_t state = url_state_scheme_start;

  utf8_string_t buffer;
  utf8_string_init(&buffer);

  err = utf8_string_reserve(&url->href, input.len);
  if (err < 0) goto err;

  bool at_sign_seen = false, inside_brackets = false, password_token_seen = false;

  for (size_t pointer = 0, n = input.len; pointer <= n; pointer++) {
    int16_t c = pointer < n ? input.data[pointer] : -1;

    switch (state) {
    // https://url.spec.whatwg.org/#scheme-start-state
    case url_state_scheme_start:
      if (url__is_ascii_alpha(c)) {
        state = url_state_scheme;
      } else {
        state = url_state_no_scheme;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#scheme-state
    case url_state_scheme:
      while (url__is_ascii_alphanumeric(c) || c == 0x2b || c == 0x2d || c == 0x2e) {
        c = pointer < n ? input.data[++pointer] : -1;
      }

      if (c == 0x3a) {
        url_type_t type = url__type(utf8_string_view_substring(input, 0, pointer));

        err = utf8_string_append_view(&url->href, utf8_string_view_substring(input, 0, pointer + 1 /* : */));
        if (err < 0) goto err;

        if (type == url_type_opaque) {
          for (size_t i = 0, n = url->href.len - 1 /* : */; i < n; i++) {
            url->href.data[i] = url__to_ascii_lowercase(url->href.data[i]);
          }

          type = url__type(utf8_string_substring(&url->href, 0, url->href.len - 1 /* : */));
        }

        url->type = type;

        url->components.scheme_end = url->href.len - 1 /* : */;

        utf8_string_view_t scheme = url_get_scheme(url);

        if (url->type == url_type_file) {
          err = utf8_string_append_literal(&url->href, (utf8_t *) "//", 2);
          if (err < 0) goto err;

          url->components.username_end = url->href.len;

          state = url_state_file;
        } else if (url__is_special(url)) {
          if (base != NULL && url->type == base->type) {
            assert(url__is_special(base));

            state = url_state_special_relative_or_authority;
          } else {
            state = url_state_special_authority_slashes;
          }
        } else if (pointer + 1 < input.len && input.data[pointer + 1] == 0x2f) {
          url->components.username_end = url->href.len;

          state = url_state_path_or_authority;
          pointer++;
        } else {
          url->flags |= url_has_opaque_path;

          url->components.username_end = url->href.len;
          url->components.host_start = url->href.len;
          url->components.host_end = url->href.len;
          url->components.path_start = url->href.len;

          state = url_state_opaque_path;
        }
      } else {
        state = url_state_no_scheme;
        pointer = -1;
      }
      break;

    // https://url.spec.whatwg.org/#no-scheme-state
    case url_state_no_scheme:
      if (base == NULL) goto err;

      if (base->flags & url_has_opaque_path) {
        if (c != 0x23) goto err;

        err = utf8_string_append_view(&url->href, url_get_scheme(base));
        if (err < 0) goto err;

        url->type = base->type;

        url->components.scheme_end = url->href.len;
        url->components.host_start = url->href.len;
        url->components.host_end = url->href.len;
        url->components.username_end = url->href.len;

        err = utf8_string_append_character(&url->href, ':');
        if (err < 0) goto err;

        url->components.path_start = url->href.len;

        err = utf8_string_append_view(&url->href, url_get_path(base));
        if (err < 0) goto err;

        utf8_string_view_t query = url_get_query(base);

        if (!utf8_string_view_empty(query)) {
          err = utf8_string_append_character(&url->href, '?');
          if (err < 0) goto err;

          url->components.query_start = url->href.len;

          err = utf8_string_append_view(&url->href, query);
          if (err < 0) goto err;
        } else {
          url->components.query_start = url->href.len + 1;
        }

        err = utf8_string_append_character(&url->href, '#');
        if (err < 0) goto err;

        url->components.fragment_start = url->href.len;

        state = url_state_fragment;
      } else if (base->type != url_type_file) {
        state = url_state_relative;
        pointer--;
      } else {
        err = utf8_string_append_literal(&url->href, (utf8_t *) "file", 4);
        if (err < 0) goto err;

        url->type = url_type_file;

        url->components.scheme_end = url->href.len;

        err = utf8_string_append_literal(&url->href, (utf8_t *) "://", 3);
        if (err < 0) goto err;

        url->components.username_end = url->href.len;

        state = url_state_file;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#special-relative-or-authority-state
    case url_state_special_relative_or_authority:
      if (c == 0x2f && pointer + 1 < input.len && input.data[pointer + 1] == 0x2f) {
        err = utf8_string_append_literal(&url->href, (utf8_t *) "//", 2);
        if (err < 0) goto err;

        url->components.username_end = url->href.len;

        state = url_state_special_authority_ignore_slashes;
        pointer++;
      } else {
        state = url_state_relative;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#path-or-authority-state
    case url_state_path_or_authority:
      if (c == 0x2f) {
        err = utf8_string_append_literal(&url->href, (utf8_t *) "//", 2);
        if (err < 0) goto err;

        url->components.username_end = url->href.len;

        state = url_state_authority;
      } else {
        url->components.username_end = url->href.len;
        url->components.host_start = url->href.len;
        url->components.host_end = url->href.len;
        url->components.path_start = url->href.len;

        state = url_state_path;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#relative-state
    case url_state_relative:
      assert(base->type != url_type_file);

      if (url->components.scheme_end == 0) {
        err = utf8_string_append_view(&url->href, url_get_scheme(base));
        if (err < 0) goto err;

        url->type = base->type;

        url->components.scheme_end = url->href.len;

        err = utf8_string_append_character(&url->href, ':');
        if (err < 0) goto err;
      }

      if (c == 0x2f) {
        err = utf8_string_append_literal(&url->href, (utf8_t *) "//", 2);
        if (err < 0) goto err;

        url->components.username_end = url->href.len;

        state = url_state_relative_slash;
      } else if (url__is_special(url) && c == 0x5c) {
        err = utf8_string_append_literal(&url->href, (utf8_t *) "//", 2);
        if (err < 0) goto err;

        url->components.username_end = url->href.len;

        state = url_state_relative_slash;
      } else {
        utf8_string_view_t host = url_get_host(base);

        if (!utf8_string_view_empty(host)) {
          err = utf8_string_append_literal(&url->href, (utf8_t *) "//", 2);
          if (err < 0) goto err;

          utf8_string_view_t username = url_get_username(base);

          if (!utf8_string_view_empty(username)) {
            err = utf8_string_append_view(&url->href, username);
            if (err < 0) goto err;

            url->components.username_end = url->href.len;

            utf8_string_view_t password = url_get_password(base);

            if (!utf8_string_view_empty(password)) {
              err = utf8_string_append_character(&url->href, ':');
              if (err < 0) goto err;

              err = utf8_string_append_view(&url->href, password);
              if (err < 0) goto err;
            }

            err = utf8_string_append_character(&url->href, '@');
            if (err < 0) goto err;
          } else {
            url->components.username_end = url->href.len;
          }

          url->components.host_start = url->href.len;

          err = utf8_string_append_view(&url->href, host);
          if (err < 0) goto err;

          url->components.host_end = url->href.len;

          utf8_string_view_t port = url_get_port(base);

          if (!utf8_string_view_empty(port)) {
            err = utf8_string_append_character(&url->href, ':');
            if (err < 0) goto err;

            err = utf8_string_append_view(&url->href, port);
            if (err < 0) goto err;

            url->components.port = base->components.port;
          }
        } else {
          url->components.username_end = url->href.len;
          url->components.host_start = url->href.len;
          url->components.host_end = url->href.len;
        }

        url->components.path_start = url->href.len;

        err = utf8_string_append_view(&url->href, url_get_path(base));
        if (err < 0) goto err;

        if (c == 0x3f) {
          state = url_state_query;
        } else if (c == 0x23) {
          url->components.query_start = url->href.len + 1;

          err = utf8_string_append_character(&url->href, '#');
          if (err < 0) goto err;

          url->components.fragment_start = url->href.len;

          state = url_state_fragment;
        } else if (c != -1) {
          url__shorten_path(url);

          state = url_state_path;
          pointer--;
        } else {
          utf8_string_view_t query = url_get_query(base);

          if (!utf8_string_view_empty(query)) {
            err = utf8_string_append_character(&url->href, '?');
            if (err < 0) goto err;

            url->components.query_start = url->href.len;

            err = utf8_string_append_view(&url->href, query);
            if (err < 0) goto err;
          } else {
            url->components.query_start = url->href.len + 1;
          }
        }
      }
      break;

    // https://url.spec.whatwg.org/#relative-slash-state
    case url_state_relative_slash:
      if (url__is_special(url) && (c == 0x2f || c == 0x5c)) {
        state = url_state_special_authority_ignore_slashes;
      } else if (c == 0x2f) {
        state = url_state_authority;
      } else {
        utf8_string_view_t host = url_get_host(base);

        if (!utf8_string_view_empty(host)) {
          err = utf8_string_append_literal(&url->href, (utf8_t *) "//", 2);
          if (err < 0) goto err;

          utf8_string_view_t username = url_get_username(base);

          if (!utf8_string_view_empty(username)) {
            err = utf8_string_append_view(&url->href, username);
            if (err < 0) goto err;

            url->components.username_end = url->href.len;

            utf8_string_view_t password = url_get_password(base);

            if (!utf8_string_view_empty(password)) {
              err = utf8_string_append_character(&url->href, ':');
              if (err < 0) goto err;

              err = utf8_string_append_view(&url->href, password);
              if (err < 0) goto err;
            }

            err = utf8_string_append_character(&url->href, '@');
            if (err < 0) goto err;
          } else {
            url->components.username_end = url->href.len;
          }

          url->components.host_start = url->href.len;

          err = utf8_string_append_view(&url->href, host);
          if (err < 0) goto err;

          url->components.host_end = url->href.len;

          utf8_string_view_t port = url_get_port(base);

          if (!utf8_string_view_empty(port)) {
            err = utf8_string_append_character(&url->href, ':');
            if (err < 0) goto err;

            err = utf8_string_append_view(&url->href, url_get_port(base));
            if (err < 0) goto err;
          }
        } else {
          url->components.username_end = url->href.len;
          url->components.host_start = url->href.len;
          url->components.host_end = url->href.len;
        }

        url->components.path_start = url->href.len;

        state = url_state_path;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#special-authority-slashes-state
    case url_state_special_authority_slashes:
      state = url_state_special_authority_ignore_slashes;

      if (c == 0x2f && pointer + 1 < input.len && input.data[pointer + 1] == 0x2f) {
        err = utf8_string_append_literal(&url->href, (utf8_t *) "//", 2);
        if (err < 0) goto err;

        url->components.username_end = url->href.len;

        pointer++;
      } else {
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#special-authority-ignore-slashes-state
    case url_state_special_authority_ignore_slashes:
      if (c != 0x2f && c != 0x5c) {
        state = url_state_authority;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#authority-state
    case url_state_authority:
      if (c == 0x40) {
        if (at_sign_seen) {
          err = utf8_string_prepend_literal(&buffer, (utf8_t *) "%40", 3);
          if (err < 0) goto err;
        }

        at_sign_seen = true;

        for (size_t i = 0, n = buffer.len; i < n; i++) {
          utf8_t c = buffer.data[i];

          if (c == 0x3a && !password_token_seen) {
            password_token_seen = true;

            url->components.username_end = url->href.len;

            err = utf8_string_append_character(&url->href, ':');
            if (err < 0) goto err;

            continue;
          }

          err = url__percent_encode_character(c, url__userinfo_percent_encode_set, &url->href);
          if (err < 0) goto err;
        }

        if (!password_token_seen) {
          url->components.username_end = url->href.len;
        }

        err = utf8_string_append_character(&url->href, '@');
        if (err < 0) goto err;

        utf8_string_clear(&buffer);
      } else if (
        (c == -1 || c == 0x2f || c == 0x3f || c == 0x23) ||
        (url__is_special(url) && c == 0x5c)
      ) {
        if (at_sign_seen && utf8_string_empty(&buffer)) goto err;

        pointer -= buffer.len + 1;

        utf8_string_clear(&buffer);

        state = url_state_host;
      } else {
        err = utf8_string_append_character(&buffer, c);
        if (err < 0) goto err;
      }
      break;

    // https://url.spec.whatwg.org/#host-state
    case url_state_host:
    // https://url.spec.whatwg.org/#hostname-state
    case url_state_hostname:
      if (c == 0x3a && !inside_brackets) {
        if (utf8_string_empty(&buffer)) goto err;

        uint32_t host_start = url->href.len;

        err = url__parse_host(utf8_string_view(&buffer), !url__is_special(url), &url->href);
        if (err < 0) goto err;

        url->components.host_start = host_start;
        url->components.host_end = url->href.len;

        utf8_string_clear(&buffer);

        state = url_state_port;
      } else if (
        (c == -1 || c == 0x2f || c == 0x3f || c == 0x23) ||
        (url__is_special(url) && c == 0x5c)
      ) {
        pointer--;

        if (url__is_special(url) && utf8_string_empty(&buffer)) goto err;

        uint32_t host_start = url->href.len;

        err = url__parse_host(utf8_string_view(&buffer), !url__is_special(url), &url->href);
        if (err < 0) goto err;

        url->components.host_start = host_start;
        url->components.host_end = url->href.len;

        utf8_string_clear(&buffer);

        url->components.path_start = url->href.len;

        state = url_state_path_start;
      } else {
        if (c == 0x5b) inside_brackets = true;
        else if (c == 0x5d) inside_brackets = false;

        err = utf8_string_append_character(&buffer, c);
        if (err < 0) goto err;
      }
      break;

    // https://url.spec.whatwg.org/#port-state
    case url_state_port:
      if (url__is_ascii_digit(c)) {
        err = utf8_string_append_character(&buffer, c);
        if (err < 0) goto err;
      } else if (
        (c == -1 || c == 0x2f || c == 0x3f || c == 0x23) ||
        (url__is_special(url) && c == 0x5c)
      ) {
        if (!utf8_string_empty(&buffer)) {
          uint32_t port = 0;

          for (size_t i = 0, n = buffer.len; i < n; i++) {
            port = port * 10 + (buffer.data[i] - 0x30);
          }

          if (port > UINT16_MAX) goto err;

          uint32_t default_port = url__default_port(url->type);

          if (port == default_port) {
            url->components.port = (uint32_t) -1;
          } else {
            err = utf8_string_append_character(&url->href, ':');
            if (err < 0) goto err;

            err = utf8_string_append(&url->href, &buffer);
            if (err < 0) goto err;

            url->components.port = port;
          }

          utf8_string_clear(&buffer);
        }

        url->components.path_start = url->href.len;

        state = url_state_path_start;
        pointer--;
      } else {
        goto err;
      }
      break;

    // https://url.spec.whatwg.org/#file-state
    case url_state_file:
      if (c == 0x2f || c == 0x5c) {
        state = url_state_file_slash;
      } else if (base != NULL && base->type == url_type_file) {
        url->components.host_start = url->href.len;

        err = utf8_string_append_view(&url->href, url_get_host(base));
        if (err < 0) goto err;

        url->components.host_end = url->href.len;

        url->components.path_start = url->href.len;

        err = utf8_string_append_view(&url->href, url_get_path(base));
        if (err < 0) goto err;

        if (c == 0x3f) {
          state = url_state_query;
        } else if (c == 0x23) {
          url->components.query_start = url->href.len + 1;

          err = utf8_string_append_character(&url->href, '#');
          if (err < 0) goto err;

          url->components.fragment_start = url->href.len;

          state = url_state_fragment;
        } else if (c != -1) {
          if (!url__starts_with_windows_drive_letter(utf8_string_view_substring(input, pointer, input.len))) {
            url__shorten_path(url);
          } else {
            url->href.len = url->components.path_start;
          }

          state = url_state_path;
          pointer--;
        } else {
          utf8_string_view_t query = url_get_query(base);

          if (!utf8_string_view_empty(query)) {
            err = utf8_string_append_character(&url->href, '?');
            if (err < 0) goto err;

            url->components.query_start = url->href.len;

            err = utf8_string_append_view(&url->href, query);
            if (err < 0) goto err;
          } else {
            url->components.query_start = url->href.len + 1;
          }
        }
      } else {
        state = url_state_path;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#file-slash-state
    case url_state_file_slash:
      if (c == 0x2f || c == 0x5c) {
        state = url_state_file_host;
      } else {
        if (base != NULL && base->type == url_type_file) {
          url->components.host_start = url->href.len;

          err = utf8_string_append_view(&url->href, url_get_host(base));
          if (err < 0) goto err;

          url->components.host_end = url->href.len;

          url->components.path_start = url->href.len;

          if (!url__starts_with_windows_drive_letter(utf8_string_view_substring(input, pointer, input.len))) {
            utf8_string_view_t path = url_get_path(base);

            size_t i = utf8_string_view_index_of_character(path, 1, '/');

            if (i != (size_t) -1 && url__is_normalized_windows_drive_letter(utf8_string_view_substring(path, 1, i))) {
              err = utf8_string_append_view(&url->href, utf8_string_view_substring(path, 0, i));
              if (err < 0) goto err;
            }
          }
        } else {
          url->components.path_start = url->href.len;
        }

        state = url_state_path;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#file-host-state
    case url_state_file_host:
      if (c == -1 || c == 0x2f || c == 0x5c || c == 0x3f || c == 0x23) {
        pointer--;

        if (url__is_windows_drive_letter(utf8_string_view(&buffer))) {
          url->components.host_start = url->href.len;
          url->components.host_end = url->href.len;

          url->components.path_start = url->href.len;

          state = url_state_path;
        } else {
          url->components.host_start = url->href.len;

          if (utf8_string_empty(&buffer)) {
            url->components.host_end = url->href.len;
          } else {
            err = url__parse_host(utf8_string_view(&buffer), !url__is_special(url), &url->href);
            if (err < 0) goto err;

            url->components.host_end = url->href.len;

            utf8_string_clear(&buffer);
          }

          url->components.path_start = url->href.len;

          state = url_state_path_start;
        }
      } else {
        err = utf8_string_append_character(&buffer, c);
        if (err < 0) goto err;
      }
      break;

    // https://url.spec.whatwg.org/#path-start-state
    case url_state_path_start:
      if (url__is_special(url)) {
        state = url_state_path;

        if (c != 0x2f && c != 0x5c) pointer--;
      } else if (c == 0x3f) {
        state = url_state_query;
      } else if (c == 0x23) {
        url->components.query_start = url->href.len + 1;

        err = utf8_string_append_character(&url->href, '#');
        if (err < 0) goto err;

        url->components.fragment_start = url->href.len;

        state = url_state_fragment;
      } else if (c != -1) {
        state = url_state_path;

        if (c != 0x2f) pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#path-state
    case url_state_path:
      if (
        (c == -1 || c == 0x2f) ||
        (url__is_special(url) && c == 0x5c) ||
        (c == 0x3f || c == 0x23)
      ) {
        utf8_string_view_t segment = utf8_string_view(&buffer);

        if (url__is_double_dot_path_segment(segment)) {
          url__shorten_path(url);

          if (c != 0x2f && !(url__is_special(url) && c == 0x5c)) {
            err = utf8_string_append_character(&url->href, '/');
            if (err < 0) goto err;
          }
        } else if (url__is_single_dot_path_segment(segment)) {
          if (c != 0x2f && !(url__is_special(url) && c == 0x5c)) {
            err = utf8_string_append_character(&url->href, '/');
            if (err < 0) goto err;
          }
        } else {
          if (
            url->type == url_type_file &&
            utf8_string_view_empty(url_get_path(url)) &&
            url__is_windows_drive_letter(segment)
          ) {
            buffer.data[1] = ':';
          }

          err = utf8_string_append_character(&url->href, '/');
          if (err < 0) goto err;

          err = utf8_string_append(&url->href, &buffer);
          if (err < 0) goto err;
        }

        utf8_string_clear(&buffer);

        if (c == 0x3f) {
          state = url_state_query;
        } else if (c == 0x23) {
          url->components.query_start = url->href.len + 1;

          err = utf8_string_append_character(&url->href, '#');
          if (err < 0) goto err;

          url->components.fragment_start = url->href.len;

          state = url_state_fragment;
        }
      } else {
        err = url__percent_encode_character(c, url__path_percent_encode_set, &buffer);
        if (err < 0) goto err;
      }
      break;

    // https://url.spec.whatwg.org/#cannot-be-a-base-url-path-state
    case url_state_opaque_path:
      if (c == 0x3f) {
        state = url_state_query;
      } else if (c == 0x23) {
        url->components.query_start = url->href.len + 1;

        err = utf8_string_append_character(&url->href, '#');
        if (err < 0) goto err;

        url->components.fragment_start = url->href.len;

        state = url_state_fragment;
      } else if (c != -1) {
        err = url__percent_encode_character(c, url__c0_control_percent_encode_set, &url->href);
        if (err < 0) goto err;
      }
      break;

    // https://url.spec.whatwg.org/#query-state
    case url_state_query:
      if (c == 0x23 || c == -1) {
        err = utf8_string_append_character(&url->href, '?');
        if (err < 0) goto err;

        url_character_set_t *query_percent_encode_set = url__is_special(url)
                                                          ? &url__special_query_percent_encode_set
                                                          : &url__query_percent_encode_set;

        url->components.query_start = url->href.len;

        err = url__percent_encode_string(utf8_string_view(&buffer), *query_percent_encode_set, &url->href);
        if (err < 0) goto err;

        utf8_string_clear(&buffer);

        url->components.fragment_start = url->href.len + 1;

        if (c == 0x23) {
          err = utf8_string_append_character(&url->href, '#');
          if (err < 0) goto err;

          state = url_state_fragment;
        }
      } else if (c != -1) {
        err = utf8_string_append_character(&buffer, c);
        if (err < 0) goto err;
      }
      break;

    // https://url.spec.whatwg.org/#fragment-state
    case url_state_fragment:
      // Optimization: The fragment is everything from the current position
      // until the end of the string.

      err = url__percent_encode_string(utf8_string_view_substring(input, pointer, input.len), url__fragment_percent_encode_set, &url->href);
      if (err < 0) goto err;

      goto done;
    }
  }

done:
  utf8_string_destroy(&buffer);

  return 0;

err:
  utf8_string_destroy(&buffer);

  return -1;
}

#endif // URL_PARSE_H
