#include "../include/url.h"
#include "helpers.h"

int
main () {
  // However many slashes stand between the scheme and the authority of a special
  // URL, they are all skipped, and so the host here is not empty.
  test_parse(url, "http:////example.com/foo/bar", NULL);

  test_get(url, href, "http://example.com/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "example.com");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
