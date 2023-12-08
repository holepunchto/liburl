#include "helpers.h"

int
main () {
  test_parse(base, "file:///foo/bar", NULL);

  test_parse(url, "./baz", base);

  test_component(url, href, "file:///foo/bar/baz");
  test_component(url, scheme, "file");
  test_component(url, path, "/foo/bar/baz");
}
