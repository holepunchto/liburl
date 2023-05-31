#ifndef URL_H
#define URL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <utf.h>

typedef struct url_range_s url_range_t;

typedef struct url_components_s url_components_t;

struct url_range_s {
  size_t start;
  size_t end;
};

struct url_components_s {
  url_range_t scheme;
  url_range_t host;
  url_range_t path;
};

static const url_range_t url_range_empty = {0, 0};

int
url_parse (const utf8_t *input, size_t len, const utf8_t *base, const url_components_t *components, url_components_t *result);

int
url_get_scheme (const utf8_t *input, const url_components_t *components, const utf8_t **result, size_t *len);

int
url_get_host (const utf8_t *input, const url_components_t *components, const utf8_t **result, size_t *len);

int
url_get_path (const utf8_t *input, const url_components_t *components, const utf8_t **result, size_t *len);

#ifdef __cplusplus
}
#endif

#endif // URL_H
