#include "../include/url.h"
#include "helpers.h"

int
main() {
  // A domain whose last label is a hexadecimal number ends in a number, and so is
  // an IPv4 address.
  test_parse(url, "http://0x7f000001/foo/bar", NULL);

  test_get(url, href, "http://127.0.0.1/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "127.0.0.1");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
