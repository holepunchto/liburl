#include "helpers.h"

int
main () {
  test_parse(base, "http://example.com/foo/bar", NULL);

  test_parse(url, "./baz", base);

  test_component(url, href, "http://example.com/foo/bar/baz");
  test_component(url, scheme, "http");
  test_component(url, host, "example.com");
  test_component(url, path, "/foo/bar/baz");
}
