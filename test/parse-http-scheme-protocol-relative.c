#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "http://example.com/foo/bar", NULL);

  test_parse(url, "//example.org/baz", &base);

  test_get(url, href, "http://example.org/baz");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "example.org");
  test_get(url, port, "");
  test_get(url, path, "/baz");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
