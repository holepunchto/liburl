#ifndef URL_H
#define URL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <utf.h>

typedef struct url_s url_t;

int
url_destroy (url_t *url);

int
url_parse (const utf8_t *input, size_t len, const url_t *base, url_t **result);

int
url_get_scheme (const url_t *url, const utf8_t **result, size_t *len);

int
url_get_host (const url_t *url, const utf8_t **result, size_t *len);

int
url_get_path (const url_t *url, const utf8_t **result, size_t *len);

#ifdef __cplusplus
}
#endif

#endif // URL_H
