#include "helpers.h"

int
main () {
  test_parse(url, "file:///foo/bar?baz", NULL);

  test_component(url, href, "file:///foo/bar?baz");
  test_component(url, scheme, "file");
  test_component(url, username, "");
  test_component(url, password, "");
  test_component(url, host, "");
  test_component(url, port, "");
  test_component(url, path, "/foo/bar");
  test_component(url, query, "baz");
  test_component(url, fragment, "");
}
