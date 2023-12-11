#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "scheme://user@host/foo/bar", NULL);

  test_parse(url, "baz", &base);

  test_get(url, href, "scheme://user@host/foo/baz");
  test_get(url, scheme, "scheme");
  test_get(url, username, "user");
  test_get(url, password, "");
  test_get(url, host, "host");
  test_get(url, port, "");
  test_get(url, path, "/foo/baz");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
