#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>

#include "../include/url.h"
#include "infra.h"

typedef struct url_range_s url_range_t;

enum {
  url_has_opaque_path = 0x1
};

struct url_s {
  int flags;

  utf8_t *scheme;
  utf8_t *host;
  utf8_t *path;
};

struct url_range_s {
  size_t start;
  size_t end;
};

typedef enum {
  url_state_scheme_start,
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

static inline int
url_string_compare (const utf8_t *a, const utf8_t *b) {
  return strcmp((const char *) a, (const char *) b);
}

static inline int
url_string_compare_literal (const utf8_t *input, const char *literal) {
  return strcmp((const char *) input, literal);
}

static inline utf8_t *
url_string_copy (const utf8_t *input, size_t len) {
  if (len == (size_t) -1) len = strlen((const char *) input);

  utf8_t *output = malloc(len + 1 /* NULL */);

  memcpy(output, input, len);

  output[len] = 0;

  return output;
}

static inline int
url_range_slice (const utf8_t *input, url_range_t range, const utf8_t **result, size_t *len) {
  if (result == NULL || len == NULL) return -1;

  if (range.start >= range.end) {
    *result = NULL;
    *len = 0;
  } else {
    *result = &input[range.start];
    *len = range.end - range.start;
  }

  return 0;
}

static inline int
url_range_compare (const utf8_t *a_input, url_range_t a, const utf8_t *b_input, url_range_t b) {
  const utf8_t *a_slice;
  size_t a_len;
  url_range_slice(a_input, a, &a_slice, &a_len);

  const utf8_t *b_slice;
  size_t b_len;
  url_range_slice(b_input, b, &b_slice, &b_len);

  size_t len = a_len < b_len ? a_len : b_len;

  return strncmp((const char *) a_slice, (const char *) b_slice, len);
}

static inline int
url_range_compare_literal (const utf8_t *input, url_range_t range, const char *literal) {
  return url_range_compare(input, range, (const utf8_t *) literal, (url_range_t){0, strlen(literal)});
}

static inline utf8_t *
url_range_copy (const utf8_t *input, url_range_t range) {
  size_t len = range.end - range.start;

  utf8_t *output = malloc(len + 1 /*NULL*/);

  memcpy(output, &input[range.start], len);

  output[len] = 0;

  return output;
}

// https://url.spec.whatwg.org/#is-special
static inline bool
url_is_special (const url_t *url) {
  const utf8_t *scheme = url->scheme;
  size_t len = strlen((const char *) scheme);

  // ftp | file
  if (len >= 3 && scheme[0] == 'f') {
    if (len == 3) return scheme[1] == 't' && scheme[2] == 'p';
    if (len == 4) return scheme[1] == 'i' && scheme[2] == 'l' && scheme[3] == 'e';
    return false;
  }

  // http(s)
  if (len >= 4 && scheme[0] == 'h' && scheme[1] == 't' && scheme[2] == 't' && scheme[3] == 'p') {
    return len == 4 || scheme[4] == 's';
  }

  // ws(s)
  if (len >= 2 && scheme[0] == 'w' && scheme[1] == 's') {
    return len == 2 || scheme[2] == 's';
  }

  return false;
}

int
url_destroy (url_t *url) {
  if (url->scheme) free(url->scheme);
  if (url->host) free(url->host);
  if (url->path) free(url->path);

  free(url);

  return 0;
}

int
url_parse (const utf8_t *input, size_t len, const url_t *base, url_t **result) {
  if (len == (size_t) -1) len = strlen((char *) input);

  url_t *url = malloc(sizeof(url_t));

  url->flags = 0;
  url->scheme = NULL;
  url->host = NULL;
  url->path = NULL;

  url_state_t state = url_state_scheme_start;

  url_range_t buffer;

  bool at_sign_seen = false, inside_brackets = false, password_token_seen = false;

  for (size_t pointer = 0; pointer <= len; pointer++) {
    int16_t c = pointer < len ? input[pointer] : -1;

    switch (state) {
    // https://url.spec.whatwg.org/#scheme-start-state
    case url_state_scheme_start:
      if (is_ascii_alpha(c)) {
        buffer.start = pointer;
        buffer.end = pointer + 1;
        state = url_state_scheme;
      } else {
        state = url_state_no_scheme;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#scheme-state
    case url_state_scheme:
      if (is_ascii_alphanumeric(c) || c == 0x2b || c == 0x2d || c == 0x2e) {
        buffer.end = pointer + 1;
      } else if (c == 0x3a) {
        url->scheme = url_range_copy(input, buffer);

        buffer.start = buffer.end = pointer;

        if (strcmp((char *) url->scheme, "file") == 0) {
          state = url_state_file;
        } else if (url_is_special(url)) {
          if (base != NULL && url_string_compare(url->scheme, base->scheme) == 0) {
            state = url_state_special_relative_or_authority;
          } else {
            state = url_state_special_authority_slashes;
          }
        } else if (pointer + 1 < len && input[pointer + 1] == 0x2f) {
          state = url_state_path_or_authority;
          pointer++;
        } else {
          state = url_state_opaque_path;
        }
      } else {
        buffer.start = buffer.end = 0;
        state = url_state_no_scheme;
        pointer = 0;
      }
      break;

    // https://url.spec.whatwg.org/#no-scheme-state
    case url_state_no_scheme:
      if (base == NULL) return -1;

      if (base->flags & url_has_opaque_path) {
        if (c != 0x23) return -1;

        url->scheme = url_string_copy(base->scheme, -1);
      }
      break;

    // https://url.spec.whatwg.org/#special-relative-or-authority-state
    case url_state_special_relative_or_authority:
      break;

    // https://url.spec.whatwg.org/#path-or-authority-state
    case url_state_path_or_authority:
      break;

    // https://url.spec.whatwg.org/#relative-state
    case url_state_relative:
      break;

    // https://url.spec.whatwg.org/#relative-slash-state
    case url_state_relative_slash:
      break;

    // https://url.spec.whatwg.org/#special-authority-slashes-state
    case url_state_special_authority_slashes:
      break;

    // https://url.spec.whatwg.org/#special-authority-ignore-slashes-state
    case url_state_special_authority_ignore_slashes:
      break;

    // https://url.spec.whatwg.org/#authority-state
    case url_state_authority:
      break;

    // https://url.spec.whatwg.org/#host-state
    case url_state_host:
    // https://url.spec.whatwg.org/#hostname-state
    case url_state_hostname:
      break;

    // https://url.spec.whatwg.org/#port-state
    case url_state_port:
      break;

    // https://url.spec.whatwg.org/#file-state
    case url_state_file:
      if (c == 0x2f || c == 0x5c) {
        state = url_state_file_slash;
      } else if (base != NULL && url_string_compare_literal(base->scheme, "file") == 0) {
        if (c == 0x3f) {
          state = url_state_query;
        } else if (c == 0x23) {
          state = url_state_fragment;
        } else if (c != -1) {
          // TODO: Windows drive letter quirk

          state = url_state_path;
          pointer--;
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
        if (base != NULL && url_string_compare_literal(base->scheme, "file") == 0) {
          // TODO: Windows drive letter quirk
        }

        state = url_state_path;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#file-host-state
    case url_state_file_host:
      if (c == -1 || c == 0x2f || c == 0x5c || c == 0x3f || c == 0x23) {
        pointer--;

        // TODO: Windows drive letter quirk

        if (buffer.start == buffer.end) {
          url->host = NULL;
        } else {
          url->host = url_range_copy(input, buffer);

          buffer.start = buffer.end = pointer;
        }

        state = url_state_path_start;
      } else {
        buffer.end = pointer + 1;
      }
      break;

    // https://url.spec.whatwg.org/#path-start-state
    case url_state_path_start:
      buffer.start = buffer.end = pointer;

      if (url_is_special(url)) {
        state = url_state_path;

        if (c != 0x2f && c != 0x5c) pointer--;
      } else if (c == 0x3f) {
        state = url_state_query;
      } else if (c == 0x23) {
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
        (url_is_special(url) && c == 0x5c) ||
        (c == 0x3f || c == 0x23)
      ) {
        if (buffer.start == buffer.end) {
          url->path = NULL;
        } else {
          url->path = url_range_copy(input, buffer);
        }

        if (c == 0x3f) {
          state = url_state_query;

        } else if (c == 0x23) {
          state = url_state_fragment;
        }
      } else {
        buffer.end = pointer + 1;
      }
      break;

    // https://url.spec.whatwg.org/#cannot-be-a-base-url-path-state
    case url_state_opaque_path:
      break;

    // https://url.spec.whatwg.org/#query-state
    case url_state_query:
      break;

    // https://url.spec.whatwg.org/#fragment-state
    case url_state_fragment:
      break;
    }
  }

  *result = url;

  return 0;

err:
  url_destroy(url);

  return -1;
}

int
url_get_scheme (const url_t *url, const utf8_t **result, size_t *len) {
  *result = url->scheme;
  if (len) *len = strlen((const char *) url->scheme);
  return 0;
}

int
url_get_host (const url_t *url, const utf8_t **result, size_t *len) {
  *result = url->host;
  if (len) *len = strlen((const char *) url->host);
  return 0;
}

int
url_get_path (const url_t *url, const utf8_t **result, size_t *len) {
  *result = url->path;
  if (len) *len = strlen((const char *) url->path);
  return 0;
}
