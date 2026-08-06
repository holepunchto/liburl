#include "../include/url.h"
#include "helpers.h"

int
main() {
  test_parse(base, "scheme://user:pass@host/foo/bar", NULL);

  // An authority of its own replaces that of the base, credentials and all, and a
  // non-special URL serializes with the two slashes that precede it.
  test_parse(url, "//other/baz", &base);

  test_get(url, href, "scheme://other/baz");
  test_get(url, scheme, "scheme");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "other");
  test_get(url, port, "");
  test_get(url, path, "/baz");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
