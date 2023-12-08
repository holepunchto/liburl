#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "scheme:/?foo", NULL);

  test_component(url, href, "scheme:/?foo");
  test_component(url, scheme, "scheme");
  test_component(url, username, "");
  test_component(url, password, "");
  test_component(url, host, "");
  test_component(url, port, "");
  test_component(url, path, "/");
  test_component(url, query, "foo");
  test_component(url, fragment, "");

  url_destroy(url);
}
