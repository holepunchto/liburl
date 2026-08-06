#include "../include/url.h"
#include "helpers.h"

int
main() {
  // Only the last label decides whether a domain ends in a number, and this one
  // holds a code point that is no hexadecimal digit.
  test_parse(url, "http://0x1.example/foo/bar", NULL);

  test_get(url, href, "http://0x1.example/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "0x1.example");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
