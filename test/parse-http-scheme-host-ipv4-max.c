#include "../include/url.h"
#include "helpers.h"

int
main() {
  // The largest address fits in a single part.
  test_parse(url, "http://4294967295/foo/bar", NULL);

  test_get(url, href, "http://255.255.255.255/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "255.255.255.255");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
