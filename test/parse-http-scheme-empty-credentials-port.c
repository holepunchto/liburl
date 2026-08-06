#include "../include/url.h"
#include "helpers.h"

int
main() {
  // Dropping an empty userinfo leaves the offsets of the host and the port that
  // follow it where they belong.
  test_parse(url, "http://:@example.com:8080/foo/bar", NULL);

  test_get(url, href, "http://example.com:8080/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "example.com");
  test_get(url, port, "8080");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
