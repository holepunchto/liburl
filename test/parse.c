#include <assert.h>
#include <string.h>
#include <utf.h>

#include "../include/url.h"

#define test_parse(url, input, base) \
  url_t *url; \
  { \
    int e = url_parse((utf8_t *) input, -1, NULL, &url); \
    assert(e == 0); \
  }

#define test_component(url, component, expected) \
  const utf8_t *component; \
  { \
    int e = url_get_##component(url, &component, NULL); \
    assert(e == 0); \
    assert(strcmp((const char *) component, expected) == 0); \
  }

int
main () {
  {
    test_parse(url, "file://", NULL);
    test_component(url, scheme, "file");
  }
  {
    test_parse(url, "file:///foo", NULL);
    test_component(url, scheme, "file");
    test_component(url, path, "/foo");
  }
  {
    test_parse(url, "file:///foo/bar", NULL);
    test_component(url, scheme, "file");
    test_component(url, path, "/foo/bar");
  }
}
