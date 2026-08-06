#include "../include/url.h"
#include "helpers.h"

int
main () {
  // The host follows the last commercial at, every earlier one belonging to the
  // userinfo and so being percent encoded.
  test_parse(url, "http://foo@bar@example.com/baz", NULL);

  test_get(url, href, "http://foo%40bar@example.com/baz");
  test_get(url, scheme, "http");
  test_get(url, username, "foo%40bar");
  test_get(url, password, "");
  test_get(url, host, "example.com");
  test_get(url, port, "");
  test_get(url, path, "/baz");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
