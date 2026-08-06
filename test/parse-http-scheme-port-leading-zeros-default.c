#include "../include/url.h"
#include "helpers.h"

int
main () {
  // A port that is the default one for the scheme is left out, its leading zeros
  // making no difference to its value.
  test_parse(url, "http://example.com:00080/foo/bar", NULL);

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
