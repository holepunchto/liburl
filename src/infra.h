#ifndef URL_INFRA_H
#define URL_INFRA_H

#include <stdbool.h>
#include <utf.h>

// https://infra.spec.whatwg.org/#ascii-digit
static inline bool
is_ascii_digit (utf8_t c) {
  return c >= 0x30 && c <= 0x39;
}

// https://infra.spec.whatwg.org/#ascii-upper-alpha
static inline bool
is_ascii_upper_alpha (utf8_t c) {
  return c >= 0x41 && c <= 0x5a;
}

// https://infra.spec.whatwg.org/#ascii-lower-alpha
static inline bool
is_ascii_lower_alpha (utf8_t c) {
  return c >= 0x61 && c <= 0x7a;
}

// https://infra.spec.whatwg.org/#ascii-alpha
static inline bool
is_ascii_alpha (utf8_t c) {
  return is_ascii_upper_alpha(c) || is_ascii_lower_alpha(c);
}

// https://infra.spec.whatwg.org/#ascii-alphanumeric
static inline bool
is_ascii_alphanumeric (utf8_t c) {
  return is_ascii_digit(c) || is_ascii_alpha(c);
}

#endif // URL_INFRA_H
