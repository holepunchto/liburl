#include "../include/url.h"
#include "helpers.h"

int
main() {
  test_parse(base, "http://:pass@example.com/foo/bar", NULL);

  // The credentials of the base are carried over on the strength of its password
  // alone, its username being empty.
  test_parse(url, "/baz/qux", &base);

  test_get(url, href, "http://:pass@example.com/baz/qux");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "pass");
  test_get(url, host, "example.com");
  test_get(url, port, "");
  test_get(url, path, "/baz/qux");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
