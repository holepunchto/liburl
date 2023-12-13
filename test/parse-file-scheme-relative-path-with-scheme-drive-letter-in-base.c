#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "file:///c:/foo/bar", NULL);

  test_parse(url, "file:/baz/quux", &base);

  test_get(url, href, "file:///c:/baz/quux");
  test_get(url, scheme, "file");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "");
  test_get(url, port, "");
  test_get(url, path, "/c:/baz/quux");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
