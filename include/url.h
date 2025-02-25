#ifndef URL_H
#define URL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <utf.h>
#include <utf/string.h>

typedef struct url_s url_t;
typedef uint32_t url_component_t;

#define url_component_unset ((url_component_t) - 1);

enum {
  url_has_opaque_path = 0x1,
};

typedef enum {
  url_type_opaque,
  url_type_http,
  url_type_https,
  url_type_ws,
  url_type_wss,
  url_type_ftp,
  url_type_file,
} url_type_t;

struct url_s {
  uint8_t flags;

  url_type_t type;

  /**
   * The normalized serialization of the URL. This is used as the canonical
   * representation to both minimize memory allocation and also provide fast,
   * immutable views over the various URL components.
   */
  utf8_string_t href;

  /**
   * https://user:pass@example.com:1234/foo/bar?baz#quux
   *      |      |     |          |^^^^|        |   |
   *      |      |     |          ||   |        |   `---- fragment_start
   *      |      |     |          ||   |        `-------- query_start
   *      |      |     |          ||   `----------------- path_start
   *      |      |     |          |`--------------------- port
   *      |      |     |          `---------------------- host_end
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

inline void
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

inline void
url_destroy (url_t *url) {
  utf8_string_destroy(&url->href);
}

inline utf8_string_view_t
url_get_href (const url_t *url) {
  return utf8_string_substring(&url->href, 0, url->href.len);
}

inline utf8_string_view_t
url_get_scheme (const url_t *url) {
  return utf8_string_substring(&url->href, 0, url->components.scheme_end);
}

inline utf8_string_view_t
url_get_username (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.scheme_end + 3 /* :// */, url->components.username_end);
}

inline utf8_string_view_t
url_get_password (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.username_end + 1 /* : */, url->components.host_start - 1 /* @ */);
}

inline utf8_string_view_t
url_get_host (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.host_start, url->components.host_end);
}

inline utf8_string_view_t
url_get_port (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.host_end + 1 /* : */, url->components.path_start);
}

inline utf8_string_view_t
url_get_path (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.path_start, url->components.query_start - 1 /* ? */);
}

inline utf8_string_view_t
url_get_query (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.query_start, url->components.fragment_start - 1 /* # */);
}

inline utf8_string_view_t
url_get_fragment (const url_t *url) {
  return utf8_string_substring(&url->href, url->components.fragment_start, url->href.len);
}

#include "url/parse.h"

int
url_parse (url_t *url, const utf8_t *input, size_t len, const url_t *base) {
  if (len == (size_t) -1) len = strlen((char *) input);

  return url__parse(url, utf8_string_view_init(input, len), base);
}

#ifdef __cplusplus
}
#endif

#endif // URL_H
