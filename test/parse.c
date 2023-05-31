#include <assert.h>
#include <string.h>
#include <utf.h>

#include "../include/url.h"

#define test_parse(input, components) \
  { \
    size_t len = strlen(input); \
    url_components_t actual; \
    int err = url_parse((utf8_t *) input, len, NULL, NULL, &actual); \
    assert(err == 0); \
    url_components_t expected = components; \
\
    assert(expected.scheme.start == actual.scheme.start); \
    assert(expected.scheme.end == actual.scheme.end); \
\
    assert(expected.host.start == actual.host.start); \
    assert(expected.host.end == actual.host.end); \
\
    assert(expected.path.start == actual.path.start); \
    assert(expected.path.end == actual.path.end); \
  }

int
main () {
  test_parse("file://", ((url_components_t){.scheme = {0, 4}}));

  test_parse("file:///foo", ((url_components_t){.scheme = {0, 4}, .path = {7, 11}}));

  test_parse("file:///foo/bar", ((url_components_t){.scheme = {0, 4}, .path = {7, 15}}));
}
