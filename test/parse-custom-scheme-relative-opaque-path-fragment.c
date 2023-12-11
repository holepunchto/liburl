#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "scheme:foo/bar", NULL);

  test_parse(url, "#baz", &base);

  test_component(url, href, "scheme:foo/bar#baz");
  test_component(url, scheme, "scheme");
  test_component(url, username, "");
  test_component(url, password, "");
  test_component(url, host, "");
  test_component(url, port, "");
  test_component(url, path, "foo/bar");
  test_component(url, query, "");
  test_component(url, fragment, "baz");

  url_destroy(&url);
}
