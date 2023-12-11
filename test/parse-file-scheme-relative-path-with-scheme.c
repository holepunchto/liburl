#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "file:///foo/bar", NULL);

  test_parse(url, "file:./baz", &base);

  test_get(url, href, "file:///foo/baz");
  test_get(url, scheme, "file");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "");
  test_get(url, port, "");
  test_get(url, path, "/foo/baz");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
