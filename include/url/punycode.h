#ifndef URL_PUNYCODE_H
#define URL_PUNYCODE_H

#include <stdbool.h>
#include <stdint.h>
#include <utf.h>
#include <utf/string.h>

#include "code-point.h"

/**
 * The bootstring parameters for Punycode.
 *
 * https://www.rfc-editor.org/rfc/rfc3492#section-5
 */
enum {
  url__punycode_base = 36,
  url__punycode_tmin = 1,
  url__punycode_tmax = 26,
  url__punycode_skew = 38,
  url__punycode_damp = 700,
  url__punycode_initial_bias = 72,
  url__punycode_initial_n = 0x80,
};

// https://www.rfc-editor.org/rfc/rfc3492#section-5
static inline utf32_t
url__punycode_encode_digit (uint32_t digit) {
  // 0..25 map to "a".."z" and 26..35 map to "0".."9".
  return digit + (digit < 26 ? 'a' : '0' - 26);
}

// https://www.rfc-editor.org/rfc/rfc3492#section-5
static inline uint32_t
url__punycode_decode_digit (utf32_t c) {
  if (c >= '0' && c <= '9') return c - '0' + 26;
  if (c >= 'a' && c <= 'z') return c - 'a';
  if (c >= 'A' && c <= 'Z') return c - 'A';

  return url__punycode_base;
}

// https://www.rfc-editor.org/rfc/rfc3492#section-6.1
static inline uint32_t
url__punycode_adapt (uint32_t delta, uint32_t points, bool first) {
  delta = first ? delta / url__punycode_damp : delta / 2;

  delta += delta / points;

  uint32_t k = 0;

  while (delta > ((url__punycode_base - url__punycode_tmin) * url__punycode_tmax) / 2) {
    delta /= url__punycode_base - url__punycode_tmin;

    k += url__punycode_base;
  }

  return k + (((url__punycode_base - url__punycode_tmin + 1) * delta) / (delta + url__punycode_skew));
}

// https://www.rfc-editor.org/rfc/rfc3492#section-6.1
static inline uint32_t
url__punycode_threshold (uint32_t k, uint32_t bias) {
  if (k <= bias + url__punycode_tmin) return url__punycode_tmin;
  if (k >= bias + url__punycode_tmax) return url__punycode_tmax;

  return k - bias;
}

/**
 * Decodes the extended string `input`, which must consist entirely of basic
 * code points, appending the decoded code points to `result`.
 *
 * https://www.rfc-editor.org/rfc/rfc3492#section-6.2
 */
static inline int
url__punycode_decode (const utf32_t *input, size_t len, url_code_points_t *result) {
  int err;

  size_t written = result->len;

  // Everything up to and including the last delimiter, if any, is the literal
  // portion of the input.
  size_t literal = 0;

  for (size_t i = 0; i < len; i++) {
    if (input[i] == '-') literal = i;
  }

  for (size_t i = 0; i < literal; i++) {
    if (input[i] >= url__punycode_initial_n) return -1;

    err = url__code_points_append(result, input[i]);
    if (err < 0) return err;
  }

  uint32_t n = url__punycode_initial_n, bias = url__punycode_initial_bias, i = 0;

  size_t pointer = literal > 0 ? literal + 1 : 0;

  while (pointer < len) {
    uint32_t previous = i, weight = 1;

    for (uint32_t k = url__punycode_base;; k += url__punycode_base) {
      if (pointer == len) return -1;

      uint32_t digit = url__punycode_decode_digit(input[pointer++]);

      if (digit >= url__punycode_base) return -1;

      if (digit > (UINT32_MAX - i) / weight) return -1;

      i += digit * weight;

      uint32_t t = url__punycode_threshold(k, bias);

      if (digit < t) break;

      if (weight > UINT32_MAX / (url__punycode_base - t)) return -1;

      weight *= url__punycode_base - t;
    }

    // The number of code points decoded so far, including the one about to be
    // inserted.
    uint32_t points = (uint32_t) (result->len - written) + 1;

    bias = url__punycode_adapt(i - previous, points, previous == 0);

    if (i / points > UINT32_MAX - n) return -1;

    n += i / points;
    i %= points;

    if (n > 0x10ffff || (n >= 0xd800 && n <= 0xdfff)) return -1;

    err = url__code_points_insert(result, written + i, n);
    if (err < 0) return err;

    i++;
  }

  return 0;
}

/**
 * Encodes `input` as an extended string, appending it to `result`. As an
 * extended string consists only of basic code points, it is appended as is.
 *
 * https://www.rfc-editor.org/rfc/rfc3492#section-6.3
 */
static inline int
url__punycode_encode (const utf32_t *input, size_t len, utf8_string_t *result) {
  int err;

  size_t basic = 0;

  for (size_t i = 0; i < len; i++) {
    if (input[i] < url__punycode_initial_n) {
      basic++;

      err = utf8_string_append_character(result, (utf8_t) input[i]);
      if (err < 0) return err;
    }
  }

  if (basic > 0) {
    err = utf8_string_append_character(result, '-');
    if (err < 0) return err;
  }

  uint32_t n = url__punycode_initial_n, bias = url__punycode_initial_bias, delta = 0;

  // The smallest code point in the input that has yet to be handled.
  uint32_t m = UINT32_MAX;

  for (size_t i = 0; i < len; i++) {
    if (input[i] >= n && input[i] < m) m = input[i];
  }

  for (size_t handled = basic; handled < len;) {
    if (m - n > (UINT32_MAX - delta) / (uint32_t) (handled + 1)) return -1;

    delta += (m - n) * (uint32_t) (handled + 1);

    n = m;

    // The code point to handle once this one is done, found along the way so
    // that each round over the input takes a single pass.
    m = UINT32_MAX;

    for (size_t i = 0; i < len; i++) {
      if (input[i] > n) {
        if (input[i] < m) m = input[i];
      } else if (input[i] < n) {
        if (delta == UINT32_MAX) return -1;

        delta++;
      } else {
        uint32_t q = delta;

        for (uint32_t k = url__punycode_base;; k += url__punycode_base) {
          uint32_t t = url__punycode_threshold(k, bias);

          if (q < t) break;

          err = utf8_string_append_character(result, (utf8_t) url__punycode_encode_digit(t + (q - t) % (url__punycode_base - t)));
          if (err < 0) return err;

          q = (q - t) / (url__punycode_base - t);
        }

        err = utf8_string_append_character(result, (utf8_t) url__punycode_encode_digit(q));
        if (err < 0) return err;

        bias = url__punycode_adapt(delta, (uint32_t) handled + 1, handled == basic);

        delta = 0;

        handled++;
      }
    }

    delta++;
    n++;
  }

  return 0;
}

#endif // URL_PUNYCODE_H
