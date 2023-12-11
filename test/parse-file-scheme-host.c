#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "file://host/foo/bar", NULL);

  test_get(url, href, "file://host/foo/bar");
  test_get(url, scheme, "file");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "host");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
