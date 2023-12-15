#ifndef URL_CHARACTER_SET
#define URL_CHARACTER_SET

#include <stdbool.h>
#include <stdint.h>
#include <utf.h>
#include <utf/string.h>

/**
 * A character set is stored as a 32 x 8 bit list with each bit
 * representing an ASCII byte. A given ASCII byte is in the character set
 * if its corresponding bit is set.
 */
typedef const uint8_t url_character_set_t[32];

static inline bool
url__is_in_character_set (url_character_set_t character_set, utf8_t character) {
  return (character_set[character >> 3] & (1 << (character & 7))) != 0;
}

static inline bool
url__contains_from_character_set (url_character_set_t character_set, utf8_string_view_t input) {
  size_t i = 0, n = input.len;

  uint8_t accumulator = 0;

  for (; i + 4 <= n; i += 4) {
    accumulator |= url__is_in_character_set(character_set, input.data[i]);
    accumulator |= url__is_in_character_set(character_set, input.data[i + 1]);
    accumulator |= url__is_in_character_set(character_set, input.data[i + 2]);
    accumulator |= url__is_in_character_set(character_set, input.data[i + 3]);
  }

  for (; i < n; i++) {
    accumulator |= url__is_in_character_set(character_set, input.data[i]);
  }

  return accumulator;
}

#endif // URL_CHARACTER_SET
