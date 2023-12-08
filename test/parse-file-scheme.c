#include "helpers.h"

int
main () {
  test_parse(url, "file:///foo/bar", NULL);

  test_component(url, href, "file:///foo/bar");
  test_component(url, scheme, "file");
  test_component(url, path, "/foo/bar");
}
