#include "../include/url.h"
#include "helpers.h"

int
main () {
  // Only the first colon of the userinfo separates the password from the username,
  // every later one being percent encoded, as is every commercial at but the last.
  test_parse(url, "http://foo:bar@baz:quux@example.com/corge", NULL);

  test_get(url, href, "http://foo:bar%40baz%3Aquux@example.com/corge");
  test_get(url, scheme, "http");
  test_get(url, username, "foo");
  test_get(url, password, "bar%40baz%3Aquux");
  test_get(url, host, "example.com");
  test_get(url, port, "");
  test_get(url, path, "/corge");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
