#include "../include/url.h"
#include "helpers.h"

int
main() {
  // Only the first colon separates the username from the password, so a second one
  // is percent encoded into a password that is not empty after all.
  test_parse(url, "http://::@example.com/foo/bar", NULL);

  test_get(url, href, "http://:%3A@example.com/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "%3A");
  test_get(url, host, "example.com");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
