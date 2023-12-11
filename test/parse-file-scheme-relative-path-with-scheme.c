#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "file:///foo/bar", NULL);

  test_parse(url, "file:./baz", &base);

  test_component(url, href, "file:///foo/baz");
  test_component(url, scheme, "file");
  test_component(url, username, "");
  test_component(url, password, "");
  test_component(url, host, "");
  test_component(url, port, "");
  test_component(url, path, "/foo/baz");
  test_component(url, query, "");
  test_component(url, fragment, "");

  url_destroy(&url);
}
