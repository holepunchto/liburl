#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "scheme:/?foo", NULL);

  test_get(url, href, "scheme:/?foo");
  test_get(url, scheme, "scheme");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "");
  test_get(url, port, "");
  test_get(url, path, "/");
  test_get(url, query, "foo");
  test_get(url, fragment, "");

  url_destroy(&url);
}
