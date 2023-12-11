#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "scheme:foo/bar?baz", NULL);

  test_parse(url, "#quux", &base);

  test_component(url, href, "scheme:foo/bar?baz#quux");
  test_component(url, scheme, "scheme");
  test_component(url, username, "");
  test_component(url, password, "");
  test_component(url, host, "");
  test_component(url, port, "");
  test_component(url, path, "foo/bar");
  test_component(url, query, "baz");
  test_component(url, fragment, "quux");

  url_destroy(&url);
}
