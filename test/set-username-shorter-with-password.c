#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "http://usr:pass@example.com:1234/foo/bar?baz#quux", NULL);

  test_set(url, username, "usr");

  test_get(url, href, "http://usr:pass@example.com:1234/foo/bar?baz#quux");
  test_get(url, scheme, "http");
  test_get(url, username, "usr");
  test_get(url, password, "pass");
  test_get(url, host, "example.com");
  test_get(url, port, "1234");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "baz");
  test_get(url, fragment, "quux");

  url_destroy(&url);
}
