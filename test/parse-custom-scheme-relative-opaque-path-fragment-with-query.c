#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "scheme:foo/bar?baz", NULL);

  test_parse(url, "#quux", &base);

  test_get(url, href, "scheme:foo/bar?baz#quux");
  test_get(url, scheme, "scheme");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "");
  test_get(url, port, "");
  test_get(url, path, "foo/bar");
  test_get(url, query, "baz");
  test_get(url, fragment, "quux");

  url_destroy(&url);
}
