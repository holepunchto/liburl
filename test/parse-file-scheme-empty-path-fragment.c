#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "file://#foo", NULL);

  test_component(url, href, "file:///#foo");
  test_component(url, scheme, "file");
  test_component(url, username, "");
  test_component(url, password, "");
  test_component(url, host, "");
  test_component(url, port, "");
  test_component(url, path, "/");
  test_component(url, query, "");
  test_component(url, fragment, "foo");

  url_destroy(url);
}
