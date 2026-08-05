#include "../include/url.h"
#include "helpers.h"

int
main () {
  // The port is serialized from its value, and so without its leading zeros.
  test_parse(url, "http://example.com:0081/foo/bar", NULL);

  test_get(url, href, "http://example.com:81/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "example.com");
  test_get(url, port, "81");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
