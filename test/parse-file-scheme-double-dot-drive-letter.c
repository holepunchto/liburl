#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "file:///c:/../foo", NULL);

  test_get(url, href, "file:///c:/foo");
  test_get(url, scheme, "file");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "");
  test_get(url, port, "");
  test_get(url, path, "/c:/foo");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
