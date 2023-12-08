#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utf.h>
#include <utf/string.h>

#include "../include/url.h"
#include "infra.h"

typedef uint32_t url_component_t;

static const url_component_t url_component_unset = (url_component_t) -1;

enum {
  url_has_opaque_path = 0x1
};

struct url_s {
  int flags;
  utf8_string_t buffer;

  /**
   * https://user:pass@example.com:1234/foo/bar?baz#quux
   *      |      |     |         | ^^^^|        |   |
   *      |      |     |         | |   |        |   `---- fragment_start
   *      |      |     |         | |   |        `-------- query_start
   *      |      |     |         | |   `----------------- path_start
   *      |      |     |         | `--------------------- port
   *      |      |     |         `----------------------- host_end
   *      |      |     `--------------------------------- host_start
   *      |      `--------------------------------------- username_end
   *      `---------------------------------------------- scheme_end
   */
  struct {
    url_component_t scheme_end;
    url_component_t username_end;
    url_component_t host_start;
    url_component_t host_end;
    url_component_t port;
    url_component_t path_start;
    url_component_t query_start;
    url_component_t fragment_start;
  } components;
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

// https://url.spec.whatwg.org/#default-port
static inline uint32_t
url_default_port (const utf8_string_view_t scheme) {
  size_t len = scheme.len;
  const utf8_t *data = scheme.data;

  // ftp | file
  if (len >= 3 && data[0] == 'f') {
    if (len == 3 && data[1] == 't' && data[2] == 'p') return 21;
    return (uint32_t) -1;
  }

  // http(s)
  if (len >= 4 && data[0] == 'h' && data[1] == 't' && data[2] == 't' && data[3] == 'p') {
    if (len == 4) return 80;
    if (len == 5 && data[4] == 's') return 443;
    return (uint32_t) -1;
  }

  // ws(s)
  if (len >= 2 && data[0] == 'w' && data[1] == 's') {
    if (len == 2) return 80;
    if (len == 3 && data[2] == 's') return 443;
    return (uint32_t) -1;
  }

  return (uint32_t) -1;
}

// https://url.spec.whatwg.org/#special-scheme
static inline bool
url_is_special_scheme (const utf8_string_view_t scheme) {
  size_t len = scheme.len;
  const utf8_t *data = scheme.data;

  // ftp | file
  if (len >= 3 && data[0] == 'f') {
    if (len == 3) return data[1] == 't' && data[2] == 'p';
    if (len == 4) return data[1] == 'i' && data[2] == 'l' && data[3] == 'e';
    return false;
  }

  // http(s)
  if (len >= 4 && data[0] == 'h' && data[1] == 't' && data[2] == 't' && data[3] == 'p') {
    if (len == 4) return true;
    if (len == 5) return data[4] == 's';
    return false;
  }

  // ws(s)
  if (len >= 2 && data[0] == 'w' && data[1] == 's') {
    if (len == 2) return true;
    if (len == 3) return data[2] == 's';
    return false;
  }

  return false;
}

// https://url.spec.whatwg.org/#is-special
static inline bool
url_is_special (const url_t *url) {
  return url_is_special_scheme(url_get_scheme(url));
}

// https://url.spec.whatwg.org/#single-dot-path-segment
static inline bool
url_is_single_dot_path_segment (const utf8_string_view_t path) {
  size_t len = path.len;
  const utf8_t *data = path.data;

  // .
  if (len == 1) return data[0] == '.';

  // %2e
  return len == 3 && data[0] == '%' && data[1] == '2' && to_ascii_lowercase(data[2]) == 'e';
}

// https://url.spec.whatwg.org/#double-dot-path-segment
static inline bool
url_is_double_dot_path_segment (const utf8_string_view_t path) {
  size_t len = path.len;
  const utf8_t *data = path.data;

  // ..
  if (len == 2 && data[0] == '.' && data[0] == '.') {
    return true;
  }

  if (len == 4) {
    // .%2e
    if (data[0] == '.' && data[1] == '%' && data[2] == '2' && to_ascii_lowercase(data[3]) == 'e') {
      return true;
    }

    // %2e.
    if (data[0] == '%' && data[1] == '2' && to_ascii_lowercase(data[2]) == 'e' && data[3] == '.') {
      return true;
    }
  }

  // %2e%2e
  return (
    len == 6 &&
    data[0] == '%' && data[1] == '2' && to_ascii_lowercase(data[2]) == 'e' &&
    data[3] == '%' && data[4] == '2' && to_ascii_lowercase(data[5]) == 'e'
  );
}

void
url_destroy (url_t *url) {
  utf8_string_destroy(&url->buffer);

  free(url);
}

int
url_parse (const utf8_t *input, size_t len, const url_t *base, url_t **result) {
  int err;

  if (len == (size_t) -1) len = strlen((char *) input);

  url_t *url = malloc(sizeof(url_t));

  url->flags = 0;

  url->components.scheme_end = 0;
  url->components.username_end = 0;
  url->components.host_start = 0;
  url->components.host_end = 0;
  url->components.port = url_component_unset;
  url->components.path_start = 0;
  url->components.query_start = url_component_unset;
  url->components.fragment_start = url_component_unset;

  err = utf8_string_init(&url->buffer);
  if (err < 0) goto err;

  url_state_t state = url_state_scheme_start;

  utf8_string_t buffer;
  err = utf8_string_init(&buffer);
  if (err < 0) goto err;

  bool at_sign_seen = false, inside_brackets = false, password_token_seen = false;

  for (size_t pointer = 0; pointer <= len; pointer++) {
    int16_t c = pointer < len ? input[pointer] : -1;

    switch (state) {
    // https://url.spec.whatwg.org/#scheme-start-state
    case url_state_scheme_start:
      if (is_ascii_alpha(c)) {
        err = utf8_string_append_character(&buffer, to_ascii_lowercase(c));
        if (err < 0) goto err;

        state = url_state_scheme;
      } else {
        state = url_state_no_scheme;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#scheme-state
    case url_state_scheme:
      if (is_ascii_alphanumeric(c) || c == 0x2b || c == 0x2d || c == 0x2e) {
        err = utf8_string_append_character(&buffer, to_ascii_lowercase(c));
        if (err < 0) goto err;
      } else if (c == 0x3a) {
        err = utf8_string_append(&url->buffer, &buffer);
        if (err < 0) goto err;

        url->components.scheme_end = url->buffer.len;

        err = utf8_string_append_character(&url->buffer, ':');
        if (err < 0) goto err;

        utf8_string_clear(&buffer);

        if (utf8_string_view_compare_literal(url_get_scheme(url), "file", 4) == 0) {
          err = utf8_string_append_literal(&url->buffer, (utf8_t *) "//", 2);
          if (err < 0) goto err;

          url->components.username_end = url->buffer.len;

          state = url_state_file;
        } else if (url_is_special(url)) {
          if (base != NULL && utf8_string_view_compare(url_get_scheme(url), url_get_scheme(base)) == 0) {
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
        state = url_state_no_scheme;
        pointer = 0;
      }
      break;

    // https://url.spec.whatwg.org/#no-scheme-state
    case url_state_no_scheme:
      if (base == NULL) goto err;

      if (base->flags & url_has_opaque_path) {
        if (c != 0x23) goto err;

        err = utf8_string_append_view(&url->buffer, url_get_scheme(base));
        if (err < 0) goto err;

        url->components.scheme_end = url->buffer.len;

        err = utf8_string_append_character(&url->buffer, ':');
        if (err < 0) goto err;

        url->components.path_start = url->buffer.len;

        err = utf8_string_append_view(&url->buffer, url_get_path(base));
        if (err < 0) goto err;

        err = utf8_string_append_character(&url->buffer, '?');
        if (err < 0) goto err;

        url->components.query_start = url->buffer.len;

        err = utf8_string_append_view(&url->buffer, url_get_query(base));
        if (err < 0) goto err;

        err = utf8_string_append_character(&url->buffer, '#');
        if (err < 0) goto err;

        url->components.fragment_start = url->buffer.len;

        state = url_state_fragment;
      } else if (utf8_string_view_compare_literal(url_get_scheme(base), "file", 4) != 0) {
        state = url_state_relative;
        pointer--;
      } else {
        err = utf8_string_append_literal(&url->buffer, (utf8_t *) "file", 4);
        if (err < 0) goto err;

        url->components.scheme_end = url->buffer.len;

        err = utf8_string_append_literal(&url->buffer, (utf8_t *) "://", 3);
        if (err < 0) goto err;

        url->components.username_end = url->buffer.len;

        state = url_state_file;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#special-relative-or-authority-state
    case url_state_special_relative_or_authority:
      if (c == 0x2f && pointer + 1 < len && input[pointer + 1] == 0x2f) {
        err = utf8_string_append_literal(&url->buffer, (utf8_t *) "//", 2);
        if (err < 0) goto err;

        url->components.username_end = len;

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
        err = utf8_string_append_literal(&url->buffer, (utf8_t *) "//", 2);
        if (err < 0) goto err;

        url->components.username_end = url->buffer.len;

        state = url_state_authority;
      } else {
        state = url_state_path;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#relative-state
    case url_state_relative:
      assert(utf8_string_view_compare_literal(url_get_scheme(base), "file", 4) != 0);

      err = utf8_string_append_view(&url->buffer, url_get_scheme(base));
      if (err < 0) goto err;

      url->components.scheme_end = url->buffer.len;

      err = utf8_string_append_character(&url->buffer, ':');
      if (err < 0) goto err;

      if (c == 0x2f) {
        state = url_state_relative_slash;
      } else if (url_is_special(url) && c == 0x5c) {
        state = url_state_relative_slash;
      } else {
        utf8_string_view_t username = url_get_username(base);

        if (!utf8_string_view_empty(username)) {
          err = utf8_string_append_view(&url->buffer, username);
          if (err < 0) goto err;

          url->components.username_end = url->buffer.len;

          utf8_string_view_t password = url_get_password(base);

          if (!utf8_string_view_empty(password)) {
            err = utf8_string_append_character(&url->buffer, ':');
            if (err < 0) goto err;

            err = utf8_string_append_view(&url->buffer, password);
            if (err < 0) goto err;
          }

          err = utf8_string_append_character(&url->buffer, '@');
          if (err < 0) goto err;
        } else {
          url->components.username_end = url->buffer.len + 2;
        }

        err = utf8_string_append_literal(&url->buffer, (utf8_t *) "//", 2);
        if (err < 0) goto err;

        url->components.host_start = url->buffer.len;

        err = utf8_string_append_view(&url->buffer, url_get_host(base));
        if (err < 0) goto err;

        url->components.host_end = url->buffer.len;

        utf8_string_view_t port = url_get_port(base);

        if (!utf8_string_view_empty(port)) {
          err = utf8_string_append_character(&url->buffer, ':');
          if (err < 0) goto err;

          err = utf8_string_append_view(&url->buffer, port);
          if (err < 0) goto err;

          url->components.port = base->components.port;
        }

        url->components.path_start = url->buffer.len;

        err = utf8_string_append_view(&url->buffer, url_get_path(base));
        if (err < 0) goto err;

        if (c == 0x3f) {
          state = url_state_query;
        } else if (c == 0x23) {
          state = url_state_fragment;
        } else if (c != -1) {
          // TODO: Shorten path

          state = url_state_path;
          pointer--;
        } else {
          utf8_string_view_t query = url_get_query(base);

          if (!utf8_string_view_empty(query)) {
            err = utf8_string_append_character(&url->buffer, '?');
            if (err < 0) goto err;

            url->components.query_start = url->buffer.len;

            err = utf8_string_append_view(&url->buffer, url_get_query(base));
            if (err < 0) goto err;
          }
        }
      }
      break;

    // https://url.spec.whatwg.org/#relative-slash-state
    case url_state_relative_slash:
      if (url_is_special(url) && (c == 0x2f || c == 0x5c)) {
        state = url_state_special_authority_ignore_slashes;
      } else if (c == 0x2f) {
        state = url_state_authority;
      } else {
        utf8_string_view_t username = url_get_username(base);

        if (!utf8_string_view_empty(username)) {
          err = utf8_string_append_view(&url->buffer, username);
          if (err < 0) goto err;

          url->components.username_end = url->buffer.len;

          utf8_string_view_t password = url_get_password(base);

          if (!utf8_string_view_empty(password)) {
            err = utf8_string_append_character(&url->buffer, ':');
            if (err < 0) goto err;

            err = utf8_string_append_view(&url->buffer, password);
            if (err < 0) goto err;
          }

          err = utf8_string_append_character(&url->buffer, '@');
          if (err < 0) goto err;
        } else {
          url->components.username_end = url->buffer.len + 2;
        }

        err = utf8_string_append_literal(&url->buffer, (utf8_t *) "//", 2);
        if (err < 0) goto err;

        url->components.host_start = url->buffer.len;

        err = utf8_string_append_view(&url->buffer, url_get_host(base));
        if (err < 0) goto err;

        url->components.host_end = url->buffer.len;

        utf8_string_view_t port = url_get_port(base);

        if (!utf8_string_view_empty(port)) {
          err = utf8_string_append_character(&url->buffer, ':');
          if (err < 0) goto err;

          err = utf8_string_append_view(&url->buffer, url_get_port(base));
          if (err < 0) goto err;
        }

        url->components.path_start = url->buffer.len;

        state = url_state_path;
        pointer--;
      }
      break;

    // https://url.spec.whatwg.org/#special-authority-slashes-state
    case url_state_special_authority_slashes:
      state = url_state_special_authority_ignore_slashes;

      if (c == 0x2f && pointer + 1 < len && input[pointer + 1] == 0x2f) {
        err = utf8_string_append_literal(&url->buffer, (utf8_t *) "//", 2);
        if (err < 0) goto err;

        url->components.username_end = url->buffer.len;

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

            url->components.username_end = url->buffer.len;

            err = utf8_string_append_character(&url->buffer, ':');
            if (err < 0) goto err;

            continue;
          }

          // TODO Percent encode

          err = utf8_string_append_character(&url->buffer, c);
          if (err < 0) goto err;
        }

        if (!password_token_seen) {
          url->components.username_end = url->buffer.len;
        }

        err = utf8_string_append_character(&url->buffer, '@');
        if (err < 0) goto err;

        utf8_string_clear(&buffer);
      } else if (
        (c == -1 || c == 0x2f || c == 0x3f || c == 0x23) ||
        (url_is_special(url) && c == 0x5c)
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
        // TODO Host parsing

        url->components.host_start = url->buffer.len;

        err = utf8_string_append(&url->buffer, &buffer);
        if (err < 0) goto err;

        url->components.host_end = url->buffer.len;

        utf8_string_clear(&buffer);

        state = url_state_port;
      } else if (
        (c == -1 || c == 0x2f || c == 0x3f || c == 0x23) ||
        (url_is_special(url) && c == 0x5c)
      ) {
        pointer--;

        if (url_is_special(url) && utf8_string_empty(&buffer)) goto err;

        // TODO Host paring

        url->components.host_start = url->buffer.len;

        err = utf8_string_append(&url->buffer, &buffer);
        if (err < 0) goto err;

        url->components.host_end = url->buffer.len;

        utf8_string_clear(&buffer);

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
      if (is_ascii_digit(c)) {
        err = utf8_string_append_character(&buffer, c);
        if (err < 0) goto err;
      } else if (
        (c == -1 || c == 0x2f || c == 0x3f || c == 0x23) ||
        (url_is_special(url) && c == 0x5c)
      ) {
        if (!utf8_string_empty(&buffer)) {
          uint32_t port = 0;

          for (size_t i = 0, n = buffer.len; i < n; i++) {
            port = port * 10 + (buffer.data[i] - 0x30);
          }

          if (port > UINT16_MAX) goto err;

          uint32_t default_port = url_default_port(url_get_scheme(url));

          if (port == default_port) {
            url->components.port = (uint32_t) -1;
          } else {
            err = utf8_string_append_character(&url->buffer, ':');
            if (err < 0) goto err;

            err = utf8_string_append(&url->buffer, &buffer);
            if (err < 0) goto err;

            url->components.port = port;
          }

          utf8_string_clear(&buffer);
        }

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
      } else if (base != NULL && utf8_string_view_compare_literal(url_get_scheme(base), "file", 4) == 0) {
        url->components.host_start = url->buffer.len;

        err = utf8_string_append_view(&url->buffer, url_get_host(base));
        if (err < 0) goto err;

        url->components.host_end = url->buffer.len;

        url->components.path_start = url->buffer.len;

        err = utf8_string_append_view(&url->buffer, url_get_path(base));
        if (err < 0) goto err;

        if (c == 0x3f) {
          state = url_state_query;
        } else if (c == 0x23) {
          url->components.query_start = url->buffer.len + 1;

          err = utf8_string_append_character(&url->buffer, '#');
          if (err < 0) goto err;

          url->components.fragment_start = url->buffer.len;

          state = url_state_fragment;
        } else if (c != -1) {
          // TODO: Windows drive letter quirk

          state = url_state_path;
          pointer--;
        } else {
          utf8_string_view_t query = url_get_query(base);

          if (!utf8_string_view_empty(query)) {
            err = utf8_string_append_character(&url->buffer, '?');
            if (err < 0) goto err;

            url->components.query_start = url->buffer.len;

            err = utf8_string_append_view(&url->buffer, url_get_query(base));
            if (err < 0) goto err;
          } else {
            url->components.query_start = url->buffer.len;
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
        if (base != NULL && utf8_string_view_compare_literal(url_get_scheme(base), "file", 4) == 0) {
          url->components.host_start = url->buffer.len;

          err = utf8_string_append_view(&url->buffer, url_get_host(base));
          if (err < 0) goto err;

          url->components.host_end = url->buffer.len;

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

        url->components.host_start = url->buffer.len;

        if (utf8_string_empty(&buffer)) {
          url->components.host_end = url->buffer.len;
        } else {
          // TODO Host parsing

          err = utf8_string_append(&url->buffer, &buffer);
          if (err < 0) goto err;

          url->components.host_end = url->buffer.len;

          utf8_string_clear(&buffer);
        }

        state = url_state_path_start;
      } else {
        err = utf8_string_append_character(&buffer, c);
        if (err < 0) goto err;
      }
      break;

    // https://url.spec.whatwg.org/#path-start-state
    case url_state_path_start:
      url->components.path_start = url->buffer.len;

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
        utf8_string_view_t segment = utf8_string_substring(&buffer, 0, buffer.len);

        if (url_is_double_dot_path_segment(segment) && c == 0x5c) {
          // TODO Shorten path

          if (c != 0x2f && !url_is_special(url) && c != 0x5c) {
            err = utf8_string_append_character(&url->buffer, '/');
            if (err < 0) goto err;
          }
        } else if (url_is_single_dot_path_segment(segment)) {
          if (c != 0x2f && !url_is_special(url) && c != 0x5c) {
            err = utf8_string_append_character(&url->buffer, '/');
            if (err < 0) goto err;
          }
        } else {
          // TODO Windows drive letter quirk

          err = utf8_string_append_character(&url->buffer, '/');
          if (err < 0) goto err;

          err = utf8_string_append(&url->buffer, &buffer);
          if (err < 0) goto err;
        }

        utf8_string_clear(&buffer);

        if (c == 0x3f) {
          state = url_state_query;
        } else if (c == 0x23) {
          url->components.query_start = url->buffer.len + 1;

          err = utf8_string_append_character(&url->buffer, '#');
          if (err < 0) goto err;

          url->components.fragment_start = url->buffer.len;

          state = url_state_fragment;
        }
      } else {
        // TODO Percent encode

        err = utf8_string_append_character(&buffer, c);
        if (err < 0) goto err;
      }
      break;

    // https://url.spec.whatwg.org/#cannot-be-a-base-url-path-state
    case url_state_opaque_path:
      if (c == 0x3f) {
        state = url_state_query;
      } else if (c == 0x23) {
        url->components.query_start = url->buffer.len + 1;

        err = utf8_string_append_character(&url->buffer, '#');
        if (err < 0) goto err;

        url->components.fragment_start = url->buffer.len;

        state = url_state_fragment;
      } else if (c != -1) {
        // TODO Percent encode

        err = utf8_string_append_character(&url->buffer, c);
        if (err < 0) goto err;
      }
      break;

    // https://url.spec.whatwg.org/#query-state
    case url_state_query:
      if (c == 0x23 || c == -1) {
        // TODO Percent encode

        err = utf8_string_append_character(&url->buffer, '?');
        if (err < 0) goto err;

        url->components.query_start = url->buffer.len;

        err = utf8_string_append(&url->buffer, &buffer);
        if (err < 0) goto err;

        utf8_string_clear(&buffer);

        url->components.fragment_start = url->buffer.len + 1;

        if (c == 0x23) {
          err = utf8_string_append_character(&url->buffer, '#');
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
      if (c != -1) {
        // TODO Percent encode

        err = utf8_string_append_character(&url->buffer, c);
        if (err < 0) goto err;
      }
      break;
    }
  }

  *result = url;

  utf8_string_destroy(&buffer);

  return 0;

err:
  utf8_string_destroy(&buffer);

  url_destroy(url);

  return -1;
}

utf8_string_view_t
url_get_href (const url_t *url) {
  return utf8_string_substring(&url->buffer, 0, url->buffer.len);
}

utf8_string_view_t
url_get_scheme (const url_t *url) {
  return utf8_string_substring(&url->buffer, 0, url->components.scheme_end);
}

utf8_string_view_t
url_get_username (const url_t *url) {
  return utf8_string_substring(&url->buffer, url->components.scheme_end + 3 /* :// */, url->components.username_end);
}

utf8_string_view_t
url_get_password (const url_t *url) {
  return utf8_string_substring(&url->buffer, url->components.username_end + 1 /* : */, url->components.host_start - 1 /* @ */);
}

utf8_string_view_t
url_get_host (const url_t *url) {
  return utf8_string_substring(&url->buffer, url->components.host_start, url->components.host_end);
}

utf8_string_view_t
url_get_port (const url_t *url) {
  size_t start, end;

  if (url->components.port == url_component_unset) {
    start = end = 0;
  } else {
    start = url->components.host_end + 1 /* : */;
    end = url->components.path_start;
  }

  return utf8_string_substring(&url->buffer, start, end);
}

utf8_string_view_t
url_get_path (const url_t *url) {
  return utf8_string_substring(&url->buffer, url->components.path_start, url->components.query_start - 1 /* ? */);
}

utf8_string_view_t
url_get_query (const url_t *url) {
  return utf8_string_substring(&url->buffer, url->components.query_start, url->components.fragment_start - 1 /* # */);
}

utf8_string_view_t
url_get_fragment (const url_t *url) {
  return utf8_string_substring(&url->buffer, url->components.fragment_start, url->buffer.len);
}
