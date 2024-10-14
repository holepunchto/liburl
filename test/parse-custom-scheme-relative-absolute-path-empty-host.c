#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "scheme:///foo", NULL);

  test_parse(url, "/bar", &base);

  test_get(url, href, "scheme:///bar");
  test_get(url, scheme, "scheme");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "");
  test_get(url, port, "");
  test_get(url, path, "/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
