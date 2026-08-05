#ifndef URL_CODE_POINT_H
#define URL_CODE_POINT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>

/**
 * A growable sequence of Unicode code points. The domain parser operates on
 * code points rather than bytes, and so needs a buffer to hold them. This
 * mirrors `utf8_string_t` in keeping short sequences, such as those of a
 * typical domain, within an inline buffer.
 */
typedef struct {
  utf32_t *data;
  size_t len;
  union {
    size_t cap;
    utf32_t buf[64];
  };
} url_code_points_t;

static inline void
url__code_points_init (url_code_points_t *code_points) {
  code_points->data = code_points->buf;
  code_points->len = 0;
}

static inline void
url__code_points_destroy (url_code_points_t *code_points) {
  if (code_points->data != code_points->buf) free(code_points->data);
}

static inline void
url__code_points_clear (url_code_points_t *code_points) {
  code_points->len = 0;
}

static inline int
url__code_points_reserve (url_code_points_t *code_points, size_t len) {
  bool inlined = code_points->data == code_points->buf;

  size_t cap = inlined ? sizeof(code_points->buf) / sizeof(utf32_t) : code_points->cap;

  if (len <= cap) return 0;

  // Grow by a factor of two to keep appending amortized constant.
  if (len < cap * 2) len = cap * 2;

  if (len > SIZE_MAX / sizeof(utf32_t)) return -1;

  utf32_t *data;

  if (inlined) {
    data = (utf32_t *) malloc(len * sizeof(utf32_t));

    if (data == NULL) return -1;

    memcpy(data, code_points->data, code_points->len * sizeof(utf32_t));
  } else {
    data = (utf32_t *) realloc(code_points->data, len * sizeof(utf32_t));

    if (data == NULL) return -1;
  }

  code_points->data = data;
  code_points->cap = len;

  return 0;
}

static inline int
url__code_points_append (url_code_points_t *code_points, utf32_t c) {
  int err;

  err = url__code_points_reserve(code_points, code_points->len + 1);
  if (err < 0) return err;

  code_points->data[code_points->len++] = c;

  return 0;
}

static inline int
url__code_points_append_many (url_code_points_t *code_points, const utf32_t *data, size_t len) {
  int err;

  if (len > SIZE_MAX - code_points->len) return -1;

  err = url__code_points_reserve(code_points, code_points->len + len);
  if (err < 0) return err;

  memcpy(&code_points->data[code_points->len], data, len * sizeof(utf32_t));

  code_points->len += len;

  return 0;
}

static inline int
url__code_points_insert (url_code_points_t *code_points, size_t pos, utf32_t c) {
  int err;

  if (pos > code_points->len) return -1;

  err = url__code_points_reserve(code_points, code_points->len + 1);
  if (err < 0) return err;

  memmove(
    &code_points->data[pos + 1],
    &code_points->data[pos],
    (code_points->len - pos) * sizeof(utf32_t)
  );

  code_points->data[pos] = c;

  code_points->len++;

  return 0;
}

#endif // URL_CODE_POINT_H
