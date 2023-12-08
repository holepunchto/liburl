#include "helpers.h"

int
main () {
  test_parse(url, "http://user@example.com/foo/bar", NULL);

  test_component(url, href, "http://user@example.com/foo/bar");
  test_component(url, scheme, "http");
  test_component(url, username, "user");
  test_component(url, password, "");
  test_component(url, host, "example.com");
  test_component(url, port, "");
  test_component(url, path, "/foo/bar");
  test_component(url, query, "");
  test_component(url, fragment, "");
}
