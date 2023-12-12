#include <stddef.h>
#include <string.h>
#include <utf.h>
#include <utf/string.h>

#include "../include/url.h"
#include "parse.h"
#include "percent-encode-set.h"
#include "percent-encode.h"

// https://url.spec.whatwg.org/#cannot-have-a-username-password-port
static inline bool
url__cannot_have_credentials_or_port (const url_t *url) {
  return utf8_string_view_empty(url_get_host(url)) || url->type == url_type_file;
}

void
url_init (url_t *url) {
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

  utf8_string_destroy(&string);

  if (err < 0) return err;

  return 1;
}

utf8_string_view_t
url_get_username (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.scheme_end + 3 /* :// */, url->components.username_end);
}

int
url_set_username (url_t *url, const utf8_t *input, size_t len) {
  int err;

  if (url__cannot_have_credentials_or_port(url)) {
    return 0;
  }

  if (len == (size_t) -1) len = strlen((char *) input);

  // https://url.spec.whatwg.org/#set-the-username

  utf8_string_t percent_encoded;
  utf8_string_init(&percent_encoded);

  err = url__percent_encode_string(utf8_string_view_init(input, len), url__userinfo_percent_encode_set, &percent_encoded);
  if (err < 0) return 0;

  uint32_t pos = url->components.scheme_end + 3 /* :// */;

  uint32_t difference = percent_encoded.len - url->components.username_end + pos;

  if (url->components.username_end <= pos) {
    err = utf8_string_append_character(&percent_encoded, '@');
    if (err < 0) goto err;

    err = utf8_string_insert(&url->href, pos, &percent_encoded);
    if (err < 0) goto err;

    url->components.username_end += difference;

    difference += 1;
  } else {
    err = utf8_string_replace(&url->href, pos, url->components.username_end - pos, &percent_encoded);
    if (err < 0) goto err;

    url->components.username_end += difference;
  }

  url->components.host_start += difference;
  url->components.host_end += difference;
  url->components.path_start += difference;

  if (url->components.query_start != url_component_unset) {
    url->components.query_start += difference;
  }

  if (url->components.fragment_start != url_component_unset) {
    url->components.fragment_start += difference;
  }

  utf8_string_destroy(&percent_encoded);

  return 1;

err:
  utf8_string_destroy(&percent_encoded);

  return -1;
}

utf8_string_view_t
url_get_password (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.username_end + 1 /* : */, url->components.host_start - 1 /* @ */);
}

int
url_set_password (url_t *url, const utf8_t *input, size_t len) {
  int err;

  if (url__cannot_have_credentials_or_port(url)) {
    return 0;
  }

  if (len == (size_t) -1) len = strlen((char *) input);

  // https://url.spec.whatwg.org/#set-the-password

  utf8_string_t percent_encoded;
  utf8_string_init(&percent_encoded);

  err = url__percent_encode_string(utf8_string_view_init(input, len), url__userinfo_percent_encode_set, &percent_encoded);
  if (err < 0) return 0;

  uint32_t pos = url->components.username_end + 1 /* : */;

  uint32_t difference = percent_encoded.len - url->components.host_start + 1 /* @ */ + pos;

  if (url->components.host_start - 1 /* @ */ <= pos) {
    pos -= 1;

    err = utf8_string_prepend_character(&percent_encoded, ':');
    if (err < 0) goto err;

    err = utf8_string_insert(&url->href, pos, &percent_encoded);
    if (err < 0) goto err;
  } else {
    err = utf8_string_replace(&url->href, pos, url->components.host_start - 1 /* @ */ - pos, &percent_encoded);
    if (err < 0) goto err;
  }

  url->components.host_start += difference;
  url->components.host_end += difference;
  url->components.path_start += difference;

  if (url->components.query_start != url_component_unset) {
    url->components.query_start += difference;
  }

  if (url->components.fragment_start != url_component_unset) {
    url->components.fragment_start += difference;
  }

  utf8_string_destroy(&percent_encoded);

  return 1;

err:
  utf8_string_destroy(&percent_encoded);

  return -1;
}

utf8_string_view_t
url_get_host (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.host_start, url->components.host_end);
}

int
url_set_host (url_t *url, const utf8_t *input, size_t len) {
  int err;

  if (url->flags & url_has_opaque_path) {
    return 0;
  }

  if (len == (size_t) -1) len = strlen((char *) input);

  err = url__parse(url, utf8_string_view_init(input, len), NULL, url_state_host);
  if (err < 0) return err;

  return 1;
}

int
url_set_hostname (url_t *url, const utf8_t *input, size_t len) {
  int err;

  if (url->flags & url_has_opaque_path) {
    return 0;
  }

  if (len == (size_t) -1) len = strlen((char *) input);

  err = url__parse(url, utf8_string_view_init(input, len), NULL, url_state_hostname);
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

  if (url__cannot_have_credentials_or_port(url)) {
    return 0;
  }

  if (len == (size_t) -1) len = strlen((char *) input);

  if (len == 0) {
    uint32_t pos = url->components.host_end;

    uint32_t difference = pos - url->components.path_start;

    err = utf8_string_erase(&url->href, pos, url->components.path_start - pos);
    assert(err == 0);

    url->components.port = url_component_unset;

    url->components.path_start += difference;

    if (url->components.query_start != url_component_unset) {
      url->components.query_start += difference;
    }

    if (url->components.fragment_start != url_component_unset) {
      url->components.fragment_start += difference;
    }
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

  if (url->flags & url_has_opaque_path) {
    return 0;
  }

  if (len == (size_t) -1) len = strlen((char *) input);

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
