#ifndef URL_INFRA_H
#define URL_INFRA_H

#include <stdbool.h>
#include <utf.h>

#include "character-set.h"

// https://infra.spec.whatwg.org/#ascii-alphanumeric
static url_character_set_t url__ascii_alphanumeric_character_set = {
  // 00    01     02     03     04     05     06     07
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 08    09     0a     0b     0c     0d     0e     0f
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 10    11     12     13     14     15     16     17
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 18    19     1a     1b     1c     1d     1e     1f
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 20    21     22     23     24     25     26     27
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 28    29     2a     2b     2c     2d     2e     2f
  0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 30    31     32     33     34     35     36     37
  0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
  // 38    39     3a     3b     3c     3d     3e     3f
  0x01 | 0x02 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 40    41     42     43     44     45     46     47
  0x00 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
  // 48    49     4a     4b     4c     4d     4e     4f
  0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
  // 50    51     52     53     54     55     56     57
  0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
  // 58    59     5a     5b     5c     5d     5e     5f
  0x01 | 0x02 | 0x04 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00,
  // 60    61     62     63     64     65     66     67
  0x00 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
  // 68    69     6a     6b     6c     6d     6e     6f
  0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
  // 70    71     72     73     74     75     76     77
  0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80,
  // 78    79     7a
  0x01 | 0x02 | 0x04,
};

// https://infra.spec.whatwg.org/#ascii-digit
static inline bool
url__is_ascii_digit (utf8_t c) {
  return c >= 0x30 && c <= 0x39;
}

// https://infra.spec.whatwg.org/#ascii-upper-hex-digit
static inline bool
url__is_ascii_upper_hex_digit (utf8_t c) {
  return url__is_ascii_digit(c) || (c >= 0x41 && c <= 0x46);
}

// https://infra.spec.whatwg.org/#ascii-lower-hex-digit
static inline bool
url__is_ascii_lower_hex_digit (utf8_t c) {
  return url__is_ascii_digit(c) || (c >= 0x61 && c <= 0x66);
}

// https://infra.spec.whatwg.org/#ascii-hex-digit
static inline bool
url__is_ascii_hex_digit (utf8_t c) {
  return url__is_ascii_digit(c) || (c >= 0x41 && c <= 0x46) || (c >= 0x61 && c <= 0x66);
}

// https://infra.spec.whatwg.org/#ascii-upper-alpha
static inline bool
url__is_ascii_upper_alpha (utf8_t c) {
  return c >= 0x41 && c <= 0x5a;
}

// https://infra.spec.whatwg.org/#ascii-lower-alpha
static inline bool
url__is_ascii_lower_alpha (utf8_t c) {
  return c >= 0x61 && c <= 0x7a;
}

// https://infra.spec.whatwg.org/#ascii-alpha
static inline bool
url__is_ascii_alpha (utf8_t c) {
  return url__is_ascii_upper_alpha(c) || url__is_ascii_lower_alpha(c);
}

// https://infra.spec.whatwg.org/#ascii-alphanumeric
static inline bool
url__is_ascii_alphanumeric (utf8_t c) {
  return url__is_in_character_set(url__ascii_alphanumeric_character_set, c);
}

// https://infra.spec.whatwg.org/#ascii-lowercase
static inline utf8_t
url__to_ascii_lowercase (utf8_t c) {
  return c | 0x20;
}

// https://infra.spec.whatwg.org/#ascii-uppercase
static inline utf8_t
url__to_ascii_uppercase (utf8_t c) {
  return c & ~0x20;
}

#endif // URL_INFRA_H
