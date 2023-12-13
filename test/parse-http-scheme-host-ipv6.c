#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "http://[2001:0db8:0000:0000:0000:ff00:0042:8329]/foo/bar", NULL);

  test_get(url, href, "http://[2001:db8::ff00:42:8329]/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "[2001:db8::ff00:42:8329]");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
