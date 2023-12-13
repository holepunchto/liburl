#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "http://192.168.0.1/foo/bar", NULL);

  test_get(url, href, "http://192.168.0.1/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "192.168.0.1");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
