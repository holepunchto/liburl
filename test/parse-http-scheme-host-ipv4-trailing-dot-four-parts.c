#include "../include/url.h"
#include "helpers.h"

int
main() {
  // Dropping the empty last part leaves four of them, which is as many as an IPv4
  // address has, rather than the five that would be one too many.
  test_parse(url, "http://1.2.3.4./foo/bar", NULL);

  test_get(url, href, "http://1.2.3.4/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "1.2.3.4");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
