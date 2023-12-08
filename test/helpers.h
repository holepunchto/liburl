#include <assert.h>
#include <stdio.h>
#include <utf.h>
#include <utf/string.h>

#include "../include/url.h"

#define test_parse(url, input, base) \
  url_t *url; \
  { \
    int e = url_parse((utf8_t *) input, -1, base, &url); \
    assert(e == 0); \
    utf8_string_view_t href = url_get_href(url); \
    printf("%.*s\n", (int) href.len, href.data); \
  }

#define test_component(url, component, expected) \
  utf8_string_view_t component; \
  { \
    component = url_get_##component(url); \
    printf("  %s = %.*s\n", #component, (int) component.len, component.data); \
    assert(utf8_string_view_compare_literal(component, expected, -1) == 0); \
  }