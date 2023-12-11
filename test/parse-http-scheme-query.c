#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "http://example.com/foo/bar?baz", NULL);

  test_get(url, href, "http://example.com/foo/bar?baz");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "example.com");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "baz");
  test_get(url, fragment, "");

  url_destroy(&url);
}
