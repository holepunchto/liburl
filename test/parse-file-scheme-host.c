#include "helpers.h"

int
main () {
  test_parse(url, "file://host/foo/bar", NULL);

  test_component(url, href, "file://host/foo/bar");
  test_component(url, scheme, "file");
  test_component(url, username, "");
  test_component(url, password, "");
  test_component(url, host, "host");
  test_component(url, port, "");
  test_component(url, path, "/foo/bar");
  test_component(url, query, "");
  test_component(url, fragment, "");
}
