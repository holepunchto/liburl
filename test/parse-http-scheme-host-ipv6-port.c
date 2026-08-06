#include "../include/url.h"
#include "helpers.h"

int
main() {
  // Only a colon outside the brackets separates a port, those within belonging to
  // the IPv6 address itself.
  test_parse(url, "http://[2001:db8::ff00:42:8329]:8080/foo/bar", NULL);

  test_get(url, href, "http://[2001:db8::ff00:42:8329]:8080/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "[2001:db8::ff00:42:8329]");
  test_get(url, port, "8080");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
