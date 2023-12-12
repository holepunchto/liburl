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
#include "percent-encode-set.h"
#include "percent-encode.h"

typedef enum {
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

static inline url_type_t
url__type (const utf8_string_view_t scheme) {
  size_t len = scheme.len;
  const utf8_t *data = scheme.data;

  // ftp | file
  if (len >= 3 && data[0] == 'f') {
    if (len == 3 && data[1] == 't' && data[2] == 'p') return url_type_ftp;
    if (len == 4 && data[1] == 'i' && data[2] == 'l' && data[3] == 'e') return url_type_file;
    return url_type_opaque;
  }

  // http(s)
  if (len >= 4 && data[0] == 'h' && data[1] == 't' && data[2] == 't' && data[3] == 'p') {
    if (len == 4) return url_type_http;
    if (len == 5 && data[4] == 's') return url_type_https;
    return url_type_opaque;
  }

  // ws(s)
  if (len >= 2 && data[0] == 'w' && data[1] == 's') {
    if (len == 2) return url_type_ws;
    if (len == 3 && data[2] == 's') return url_type_https;
    return url_type_opaque;
  }

  return url_type_opaque;
}

// https://url.spec.whatwg.org/#default-port
static inline uint32_t
url__default_port (url_type_t type) {
  switch (type) {
  case url_type_ftp:
    return 21;
  case url_type_http:
  case url_type_ws:
    return 80;
  case url_type_https:
  case url_type_wss:
    return 443;
  default:
    return (uint32_t) -1;
  }
}

// https://url.spec.whatwg.org/#is-special
static inline bool
url__is_special (const url_t *url) {
  return url->type != url_type_opaque;
}

// https://url.spec.whatwg.org/#include-credentials
static inline bool
url__includes_credentials (const url_t *url) {
  return !utf8_string_view_empty(url_get_username(url));
}

// https://url.spec.whatwg.org/#url-opaque-path
static inline bool
url__has_opaque_path (const url_t *url) {
  return (url->flags & url_has_opaque_path) != 0;
}

// https://url.spec.whatwg.org/#cannot-have-a-username-password-port
static inline bool
url__cannot_have_username_password_port (const url_t *url) {
  return utf8_string_view_empty(url_get_host(url)) || url->type == url_type_file;
}

// https://url.spec.whatwg.org/#shorten-a-urls-path
static inline void
url__shorten_path (url_t *url) {
  assert((url->flags & url_has_opaque_path) == 0);

  // TODO Windows drive letter quick

  size_t i = utf8_string_view_last_index_of_character(url_get_path(url), '/', -1);

  if (i == (size_t) -1) return;

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

// https://url.spec.whatwg.org/#ipv4-number-parser
static inline int
url__parse_ipv4_number (const utf8_string_view_t input, utf8_string_t *result) {
  return -1;
}

// https://url.spec.whatwg.org/#concept-ipv4-parser
static inline int
url__parse_ipv4 (const utf8_string_view_t input, utf8_string_t *result) {
  // TODO

  return -1;
}

// https://url.spec.whatwg.org/#concept-ipv6-parser
static inline int
url__parse_ipv6 (const utf8_string_view_t input, utf8_string_t *result) {
  // TODO

  return -1;
}

// https://url.spec.whatwg.org/#concept-opaque-host-parser
static inline int
url__parse_opqaue_host (const utf8_string_view_t input, utf8_string_t *result) {
  // TODO Forbidden host code point

  return url__percent_encode_string(input, url__c0_control_percent_encode_set, result);
}

// https://url.spec.whatwg.org/#ends-in-a-number-checker
static inline bool
url__ends_in_a_number (const utf8_string_t *string) {
  // TODO

  return false;
}

// https://url.spec.whatwg.org/#concept-host-parser
static inline int
url__parse_host (const utf8_string_view_t input, bool is_opaque, utf8_string_t *result) {
  int err;

  if (input.len > 0 && input.data[0] == 0x5b) {
    if (input.data[input.len - 1] != 0x5d) return -1;

    return url__parse_ipv6(utf8_string_view_substring(input, 1, input.len - 1), result);
  }

  if (is_opaque) return url__parse_opqaue_host(input, result);

  assert(input.len != 0);

  utf8_string_t domain;
  utf8_string_init(&domain);

  err = url__percent_decode_string(input, &domain);
  if (err < 0) goto err;

  // TODO Domain to ASCII

  utf8_string_t ascii_domain = domain;

  if (url__ends_in_a_number(&ascii_domain)) {
    err = url__parse_ipv4_number(utf8_string_substring(&ascii_domain, 0, ascii_domain.len), result);
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
url__parse (url_t *url, const utf8_string_view_t input, const url_t *base, url_state_t state_override) {
  int err;

  if (state_override == 0) {
    url->flags = 0;
    url->type = url_type_opaque;

    url->components.scheme_end = 0;
    url->components.username_end = 0;
    url->components.host_start = 0;
    url->components.host_end = 0;
    url->components.port = url_component_unset;
    url->components.path_start = 0;
    url->components.query_start = url_component_unset;
    url->components.fragment_start = url_component_unset;

    utf8_string_init(&url->href);
  }

  url_state_t state = state_override || url_state_scheme_start;

  utf8_string_t buffer;
  utf8_string_init(&buffer);

  bool at_sign_seen = false, inside_brackets = false, password_token_seen = false;

  for (size_t pointer = 0, n = input.len; pointer <= n; pointer++) {
    int16_t c = pointer < n ? input.data[pointer] : -1;

    switch (state) {
    // https://url.spec.whatwg.org/#scheme-start-state
    case url_state_scheme_start:
      if (url__is_ascii_alpha(c)) {
        err = utf8_string_append_character(&buffer, url__to_ascii_lowercase(c));
        if (err < 0) goto err;

        state = url_state_scheme;
      } else if (state_override == 0) {
        state = url_state_no_scheme;
        pointer--;
      } else {
        goto err;
      }
      break;

    // https://url.spec.whatwg.org/#scheme-state
    case url_state_scheme:
      if (url__is_ascii_alphanumeric(c) || c == 0x2b || c == 0x2d || c == 0x2e) {
        err = utf8_string_append_character(&buffer, url__to_ascii_lowercase(c));
        if (err < 0) goto err;
      } else if (c == 0x3a) {
        url_type_t type = url__type(utf8_string_substring(&buffer, 0, buffer.len));

        bool is_special = type != url_type_opaque;

        if (state_override) {
          if (url__is_special(url) != is_special) {
            goto done;
          }

          if (url__includes_credentials(url) || url->components.port != url_component_unset) {
            if (url->type == url_type_file) {
              goto done;
            }
          }

          if (url->type == url_type_file && utf8_string_view_empty(url_get_host(url))) {
            goto done;
          }

          uint32_t difference = buffer.len - url->components.scheme_end;

          err = utf8_string_replace(&url->href, 0, url->components.scheme_end, &buffer);
          if (err < 0) goto err;

          url->type = type;

          url->components.scheme_end += difference;
          url->components.username_end += difference;
          url->components.host_start += difference;
          url->components.host_end += difference;
          url->components.path_start += difference;

          if (url->components.query_start != url_component_unset) {
            url->components.query_start += difference;
          }

          if (url->components.fragment_start != url_component_unset) {
            url->components.fragment_start += difference;
          }

          uint32_t default_port = url__default_port(type);

          if (url->components.port == default_port) {
            url->components.port = (uint32_t) -1;

            size_t len = url->components.path_start - url->components.host_end;

            err = utf8_string_erase(&url->href, url->components.host_end, len);
            assert(err == 0);

            url->components.path_start -= len;
            url->components.query_start -= len;
            url->components.fragment_start -= len;
          }

          goto done;
        }

        err = utf8_string_append(&url->href, &buffer);
        if (err < 0) goto err;

        url->type = type;

        url->components.scheme_end = url->href.len;

        err = utf8_string_append_character(&url->href, ':');
        if (err < 0) goto err;

        utf8_string_clear(&buffer);

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
      } else if (state_override == 0) {
        utf8_string_clear(&buffer);

        state = url_state_no_scheme;
        pointer = -1;
      } else {
        goto err;
      }
      break;

    // https://url.spec.whatwg.org/#no-scheme-state
    case url_state_no_scheme:
      if (base == NULL) goto err;

      if (url__has_opaque_path(base)) {
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
        url->components.host_start = url->href.len;

        err = url__parse_host(utf8_string_substring(&buffer, 0, buffer.len), !url__is_special(url), &url->href);
        if (err < 0) goto err;

        url->components.host_end = url->href.len;

        utf8_string_clear(&buffer);

        state = url_state_port;
      } else if (
        (c == -1 || c == 0x2f || c == 0x3f || c == 0x23) ||
        (url__is_special(url) && c == 0x5c)
      ) {
        pointer--;

        if (url__is_special(url) && utf8_string_empty(&buffer)) goto err;

        url->components.host_start = url->href.len;

        err = url__parse_host(utf8_string_substring(&buffer, 0, buffer.len), !url__is_special(url), &url->href);
        if (err < 0) goto err;

        url->components.host_end = url->href.len;

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
          // TODO: Windows drive letter quirk

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

        url->components.host_start = url->href.len;

        if (utf8_string_empty(&buffer)) {
          url->components.host_end = url->href.len;
        } else {
          err = url__parse_host(utf8_string_substring(&buffer, 0, buffer.len), !url__is_special(url), &url->href);
          if (err < 0) goto err;

          url->components.host_end = url->href.len;

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
      url->components.path_start = url->href.len;

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
        utf8_string_view_t segment = utf8_string_substring(&buffer, 0, buffer.len);

        if (url__is_double_dot_path_segment(segment) && c == 0x5c) {
          url__shorten_path(url);

          if (c != 0x2f && !url__is_special(url) && c != 0x5c) {
            err = utf8_string_append_character(&url->href, '/');
            if (err < 0) goto err;
          }
        } else if (url__is_single_dot_path_segment(segment)) {
          if (c != 0x2f && !url__is_special(url) && c != 0x5c) {
            err = utf8_string_append_character(&url->href, '/');
            if (err < 0) goto err;
          }
        } else {
          // TODO Windows drive letter quirk

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

        url_percent_encode_set_t *query_percent_encode_set = url__is_special(url)
                                                               ? &url__special_query_percent_encode_set
                                                               : &url__query_percent_encode_set;

        url->components.query_start = url->href.len;

        err = url__percent_encode_string(utf8_string_substring(&buffer, 0, buffer.len), *query_percent_encode_set, &url->href);
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

  url_destroy(url);

  return -1;
}

void
url_destroy (url_t *url) {
  utf8_string_destroy(&url->href);
}

int
url_parse (url_t *url, const utf8_t *input, size_t len, const url_t *base) {
  if (len == (size_t) -1) len = strlen((char *) input);

  return url__parse(url, utf8_string_view_init(input, len), base, 0);
}

utf8_string_view_t
url_get_href (const url_t *url) {
  return utf8_string_substring(&url->href, 0, url->href.len);
}

int
url_set_href (url_t *url, const utf8_t *input, size_t len) {
  int err;

  if (len == (size_t) -1) len = strlen((char *) input);

  url_t parsed_url;
  err = url__parse(&parsed_url, utf8_string_view_init(input, len), NULL, 0);
  if (err < 0) return err;

  url_destroy(url);

  *url = parsed_url;

  return 1;
}

utf8_string_view_t
url_get_scheme (const url_t *url) {
  return utf8_string_substring(&url->href, 0, url->components.scheme_end);
}

int
url_set_scheme (url_t *url, const utf8_t *input, size_t len) {
  int err;

  if (len == (size_t) -1) len = strlen((char *) input);

  utf8_string_t string;
  err = utf8_string_view_concat_character(utf8_string_view_init(input, len), ':', &string);
  if (err < 0) return err;

  err = url__parse(url, utf8_string_substring(&string, 0, string.len), NULL, url_state_scheme_start);
  if (err < 0) {
    utf8_string_destroy(&string);

    return err;
  }

  return 1;
}

utf8_string_view_t
url_get_username (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.scheme_end + 3 /* :// */, url->components.username_end);
}

int
url_set_username (url_t *url, const utf8_t *input, size_t len) {
  if (url__cannot_have_username_password_port(url)) {
    return 0;
  }

  // https://url.spec.whatwg.org/#set-the-username

  // TODO

  return 0;
}

utf8_string_view_t
url_get_password (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.username_end + 1 /* : */, url->components.host_start - 1 /* @ */);
}

int
url_set_password (url_t *url, const utf8_t *input, size_t len) {
  if (url__cannot_have_username_password_port(url)) {
    return 0;
  }

  // https://url.spec.whatwg.org/#set-the-password

  // TODO

  return 0;
}

utf8_string_view_t
url_get_host (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.host_start, url->components.host_end);
}

int
url_set_host (url_t *url, const utf8_t *input, size_t len) {
  int err;

  if (url__has_opaque_path(url)) {
    return 0;
  }

  err = url__parse(url, utf8_string_view_init(input, len), NULL, url_state_host);
  if (err < 0) return err;

  return 1;
}

utf8_string_view_t
url_get_port (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.host_end + 1 /* : */, url->components.path_start);
}

int
url_set_port (url_t *url, const utf8_t *input, size_t len) {
  int err;

  if (url__cannot_have_username_password_port(url)) {
    return 0;
  }

  if (len == (size_t) -1) len = strlen((char *) input);

  if (len == 0) {
    url->components.port = url_component_unset;

    // TODO Adjust offsets
  } else {
    err = url__parse(url, utf8_string_view_init(input, len), NULL, url_state_port);
    if (err < 0) return err;
  }

  return 1;
}

utf8_string_view_t
url_get_path (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.path_start, url->components.query_start - 1 /* ? */);
}

int
url_set_path (url_t *url, const utf8_t *input, size_t len) {
  int err;

  if (url__has_opaque_path(url)) {
    return 0;
  }

  // TODO Empty path

  err = url__parse(url, utf8_string_view_init(input, len), NULL, url_state_path_start);
  if (err < 0) return err;

  return 1;
}

utf8_string_view_t
url_get_query (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.query_start, url->components.fragment_start - 1 /* # */);
}

int
url_set_query (url_t *url, const utf8_t *input, size_t len) {
  int err;

  if (len == (size_t) -1) len = strlen((char *) input);

  // TODO Empty query

  if (len == 0) {
    // TODO Strip trailing spaces

    return 1;
  }

  err = url__parse(url, utf8_string_view_init(input, len), NULL, url_state_query);
  if (err < 0) return err;

  return 1;
}

utf8_string_view_t
url_get_fragment (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.fragment_start, url->href.len);
}

int
url_set_fragment (url_t *url, const utf8_t *input, size_t len) {
  int err;

  if (len == (size_t) -1) len = strlen((char *) input);

  // TODO Empty fragment

  if (len == 0) {
    // TODO Strip trailing spaces

    return 1;
  }

  err = url__parse(url, utf8_string_view_init(input, len), NULL, url_state_fragment);
  if (err < 0) return err;

  return 1;
}
