#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "HTTP://Example.COM/FOO/Bar", NULL);

  test_get(url, href, "http://example.com/FOO/Bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "example.com");
  test_get(url, port, "");
  test_get(url, path, "/FOO/Bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
