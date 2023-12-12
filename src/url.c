#include <stddef.h>
#include <string.h>
#include <utf.h>
#include <utf/string.h>

#include "../include/url.h"
#include "parse.h"

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

  return url__parse(url, utf8_string_view_init(input, len), base);
}

utf8_string_view_t
url_get_href (const url_t *url) {
  return utf8_string_substring(&url->href, 0, url->href.len);
}

utf8_string_view_t
url_get_scheme (const url_t *url) {
  return utf8_string_substring(&url->href, 0, url->components.scheme_end);
}

utf8_string_view_t
url_get_username (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.scheme_end + 3 /* :// */, url->components.username_end);
}

utf8_string_view_t
url_get_password (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.username_end + 1 /* : */, url->components.host_start - 1 /* @ */);
}

utf8_string_view_t
url_get_host (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.host_start, url->components.host_end);
}

utf8_string_view_t
url_get_port (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.host_end + 1 /* : */, url->components.path_start);
}

utf8_string_view_t
url_get_path (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.path_start, url->components.query_start - 1 /* ? */);
}

utf8_string_view_t
url_get_query (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.query_start, url->components.fragment_start - 1 /* # */);
}

utf8_string_view_t
url_get_fragment (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.fragment_start, url->href.len);
}
