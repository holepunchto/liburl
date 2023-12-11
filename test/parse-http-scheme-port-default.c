#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "http://example.com:80/foo/bar", NULL);

  test_component(url, href, "http://example.com/foo/bar");
  test_component(url, scheme, "http");
  test_component(url, username, "");
  test_component(url, password, "");
  test_component(url, host, "example.com");
  test_component(url, port, "");
  test_component(url, path, "/foo/bar");
  test_component(url, query, "");
  test_component(url, fragment, "");

  url_destroy(&url);
}
