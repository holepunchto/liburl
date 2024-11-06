#ifndef URL_TYPE_H
#define URL_TYPE_H

#include <utf.h>
#include <utf/string.h>

#include "../url.h"

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

#endif // URL_TYPE_H
