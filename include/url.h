#ifndef URL_H
#define URL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <utf.h>
#include <utf/string.h>

typedef struct url_s url_t;

void
url_destroy (url_t *url);

int
url_parse (const utf8_t *input, size_t len, const url_t *base, url_t **result);

utf8_string_view_t
url_get_href (const url_t *url);

utf8_string_view_t
url_get_scheme (const url_t *url);

utf8_string_view_t
url_get_username (const url_t *url);

utf8_string_view_t
url_get_password (const url_t *url);

utf8_string_view_t
url_get_host (const url_t *url);

utf8_string_view_t
url_get_port (const url_t *url);

utf8_string_view_t
url_get_path (const url_t *url);

utf8_string_view_t
url_get_query (const url_t *url);

utf8_string_view_t
url_get_fragment (const url_t *url);

#ifdef __cplusplus
}
#endif

#endif // URL_H
