#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "http://:pass@example.com/foo/bar", NULL);

  test_get(url, href, "http://:pass@example.com/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "pass");
  test_get(url, host, "example.com");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
