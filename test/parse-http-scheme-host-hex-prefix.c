#include "../include/url.h"
#include "helpers.h"

int
main() {
  // A hexadecimal prefix with no digits behind it is read as zero rather than
  // rejected, and so the domain ends in a number and is an IPv4 address.
  test_parse(url, "http://0x/foo/bar", NULL);

  test_get(url, href, "http://0.0.0.0/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "0.0.0.0");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
