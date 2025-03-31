#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "scheme://host", NULL);

  test_parse(url, "./foo", &base);

  test_get(url, href, "scheme://host/foo");
  test_get(url, scheme, "scheme");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "host");
  test_get(url, port, "");
  test_get(url, path, "/foo");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
