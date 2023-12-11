#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "scheme://user@host/foo/bar", NULL);

  test_parse(url, "baz", &base);

  test_component(url, href, "scheme://user@host/foo/baz");
  test_component(url, scheme, "scheme");
  test_component(url, username, "user");
  test_component(url, password, "");
  test_component(url, host, "host");
  test_component(url, port, "");
  test_component(url, path, "/foo/baz");
  test_component(url, query, "");
  test_component(url, fragment, "");

  url_destroy(&url);
}
