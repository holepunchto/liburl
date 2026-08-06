#ifndef URL_SERIALIZE_H
#define URL_SERIALIZE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <utf.h>
#include <utf/string.h>

/**
 * The longest serialization of an IPv4 address, that of 255.255.255.255.
 */
#define URL__IPV4_MAX_LEN 15

/**
 * The longest serialization of an IPv6 address, that of eight four digit pieces
 * with the seven colons that separate them.
 */
#define URL__IPV6_MAX_LEN 39

/**
 * The longest serialization of a port, that of 65535.
 */
#define URL__PORT_MAX_LEN 5

static const utf8_t url__hex_digits[16] = "0123456789abcdef";

// https://url.spec.whatwg.org/#url-serializing
static inline int
url__serialize_port(uint16_t port, utf8_string_t *result) {
  utf8_t buffer[URL__PORT_MAX_LEN];

  size_t len = 0;

  if (port >= 10000) buffer[len++] = '0' + port / 10000;
  if (port >= 1000) buffer[len++] = '0' + port / 1000 % 10;
  if (port >= 100) buffer[len++] = '0' + port / 100 % 10;
  if (port >= 10) buffer[len++] = '0' + port / 10 % 10;

  buffer[len++] = '0' + port % 10;

  return utf8_string_append_literal(result, buffer, len);
}

// https://url.spec.whatwg.org/#concept-ipv4-serializer
static inline int
url__serialize_ipv4(uint32_t address, utf8_string_t *result) {
  utf8_t buffer[URL__IPV4_MAX_LEN];

  size_t len = 0;

  for (uint8_t i = 4; i-- > 0;) {
    uint8_t part = (address >> (i * 8)) & 0xff;

    if (part >= 100) buffer[len++] = '0' + part / 100;
    if (part >= 10) buffer[len++] = '0' + part / 10 % 10;

    buffer[len++] = '0' + part % 10;

    if (i != 0) buffer[len++] = '.';
  }

  return utf8_string_append_literal(result, buffer, len);
}

// https://url.spec.whatwg.org/#concept-ipv6-serializer
static inline int
url__serialize_ipv6(uint16_t address[8], utf8_string_t *result) {
  uint8_t compress_length = 0;
  uint8_t compress = 8;

  for (size_t i = 0; i < 8; i++) {
    if (address[i] == 0) {
      size_t next = i + 1;

      while (next < 8 && address[next] == 0) {
        next++;
      }

      uint8_t count = next - i;

      if (count > 1 && compress_length < count) {
        compress_length = count;
        compress = i;
        if (next == 8) break;
        i = next;
      }
    }
  }

  utf8_t buffer[URL__IPV6_MAX_LEN];

  size_t len = 0;

  bool ignore_0 = false;

  for (uint8_t piece_index = 0; piece_index < 8; piece_index++) {
    if (ignore_0 && address[piece_index] == 0) continue;

    ignore_0 = false;

    if (compress == piece_index) {
      // The piece before the compressed run, if there is one, has already left a
      // colon behind it.
      if (piece_index == 0) buffer[len++] = ':';

      buffer[len++] = ':';

      ignore_0 = true;
      continue;
    }

    uint16_t piece = address[piece_index];

    if (piece >= 0x1000) buffer[len++] = url__hex_digits[piece >> 12 & 0xf];
    if (piece >= 0x100) buffer[len++] = url__hex_digits[piece >> 8 & 0xf];
    if (piece >= 0x10) buffer[len++] = url__hex_digits[piece >> 4 & 0xf];

    buffer[len++] = url__hex_digits[piece & 0xf];

    if (piece_index != 7) buffer[len++] = ':';
  }

  return utf8_string_append_literal(result, buffer, len);
}

#endif // URL_SERIALIZE_H
