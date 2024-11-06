#include <stddef.h>
#include <string.h>
#include <utf.h>
#include <utf/string.h>

#include "../include/url.h"

extern void
url_init (url_t *url);

extern void
url_destroy (url_t *url);

extern utf8_string_view_t
url_get_href (const url_t *url);

extern utf8_string_view_t
url_get_scheme (const url_t *url);

extern utf8_string_view_t
url_get_username (const url_t *url);

extern utf8_string_view_t
url_get_password (const url_t *url);

extern utf8_string_view_t
url_get_host (const url_t *url);

extern utf8_string_view_t
url_get_port (const url_t *url);

extern utf8_string_view_t
url_get_path (const url_t *url);

extern utf8_string_view_t
url_get_query (const url_t *url);

extern utf8_string_view_t
url_get_fragment (const url_t *url);

extern int
url_parse (url_t *url, const utf8_t *input, size_t len, const url_t *base);
