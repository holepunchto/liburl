#ifndef URL_SERIALIZE_H
#define URL_SERIALIZE_H

#include <stdint.h>
#include <stdio.h>
#include <utf.h>
#include <utf/string.h>

// https://url.spec.whatwg.org/#concept-ipv4-serializer
static inline int
url__serialize_ipv4 (uint32_t address, utf8_string_t *result) {
  int err;

  uint32_t n = address;

  for (uint8_t i = 3; i <= 3; i--) {
    char number[4];
    err = snprintf(number, 4, "%d", (address >> (i * 8)) & 0xff);
    assert(err > 0);

    err = utf8_string_append_literal(result, (utf8_t *) number, err);
    if (err < 0) return err;

    if (i != 0) {
      err = utf8_string_append_character(result, '.');
      if (err < 0) return err;
    }
  }

  return 0;
}

// https://url.spec.whatwg.org/#concept-ipv6-serializer
static inline int
url__serialize_ipv6 (uint16_t address[8], utf8_string_t *result) {
  int err;

  uint8_t compress_length = 0;
  uint8_t compress = 8;

  for (size_t i = 0; i < 8; i++) {
    if (address[i] == 0) {
      size_t next = i + 1;

      while (next < 8 && address[next] == 0) {
        next++;
      }

      uint8_t count = next - i;

      if (compress_length < count) {
        compress_length = count;
        compress = i;
        if (next == 8) break;
        i = next;
      }
    }
  }

  bool ignore_0 = false;

  for (uint8_t piece_index = 0; piece_index < 8; piece_index++) {
    if (ignore_0 && address[piece_index] == 0) continue;

    ignore_0 = false;

    if (compress == piece_index) {
      err = utf8_string_append_literal(result, (utf8_t *) "::", piece_index == 0 ? 2 : 1);
      if (err < 0) return err;

      ignore_0 = true;
      continue;
    }

    char number[5];
    err = snprintf(number, 5, "%x", address[piece_index]);
    assert(err > 0);

    err = utf8_string_append_literal(result, (utf8_t *) number, err);
    if (err < 0) return err;

    if (piece_index != 7) {
      err = utf8_string_append_character(result, ':');
      if (err < 0) return err;
    }
  }

  return 0;
}

#endif // URL_SERIALIZE_H
