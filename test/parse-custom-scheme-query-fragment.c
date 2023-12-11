#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "scheme:?foo#bar", NULL);

  test_get(url, href, "scheme:?foo#bar");
  test_get(url, scheme, "scheme");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "");
  test_get(url, port, "");
  test_get(url, path, "");
  test_get(url, query, "foo");
  test_get(url, fragment, "bar");

  url_destroy(&url);
}
