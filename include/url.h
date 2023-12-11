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

static const url_component_t url_component_unset = (url_component_t) -1;

enum {
  url_is_special = 0x1,
  url_has_opaque_path = 0x2,
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

void
url_destroy (url_t *url);

int
url_parse (url_t *url, const utf8_t *input, size_t len, const url_t *base);

utf8_string_view_t
url_get_href (const url_t *url);

int
url_set_href (url_t *url, const utf8_t *input, size_t len);

utf8_string_view_t
url_get_scheme (const url_t *url);

int
url_set_scheme (url_t *url, const utf8_t *input, size_t len);

utf8_string_view_t
url_get_username (const url_t *url);

int
url_set_username (url_t *url, const utf8_t *input, size_t len);

utf8_string_view_t
url_get_password (const url_t *url);

int
url_set_password (url_t *url, const utf8_t *input, size_t len);

utf8_string_view_t
url_get_host (const url_t *url);

int
url_set_host (url_t *url, const utf8_t *input, size_t len);

utf8_string_view_t
url_get_port (const url_t *url);

int
url_set_port (url_t *url, const utf8_t *input, size_t len);

utf8_string_view_t
url_get_path (const url_t *url);

int
url_set_path (url_t *url, const utf8_t *input, size_t len);

utf8_string_view_t
url_get_query (const url_t *url);

int
url_set_query (url_t *url, const utf8_t *input, size_t len);

utf8_string_view_t
url_get_fragment (const url_t *url);

int
url_set_fragment (url_t *url, const utf8_t *input, size_t len);

#ifdef __cplusplus
}
#endif

#endif // URL_H
