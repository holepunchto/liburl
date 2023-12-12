#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "http://example.com:1234/foo/bar?baz#quux", NULL);

  test_set(url, host, "example.org:80");

  test_get(url, href, "http://example.org/foo/bar?baz#quux");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "example.org");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "baz");
  test_get(url, fragment, "quux");

  url_destroy(&url);
}
