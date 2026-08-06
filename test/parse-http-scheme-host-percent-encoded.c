#include "../include/url.h"
#include "helpers.h"

int
main() {
  // A percent encoded domain is decoded before it is converted, so the escape here
  // becomes the letter it stands for.
  test_parse(url, "https://a%41b.example/foo/bar", NULL);

  test_get(url, href, "https://aab.example/foo/bar");
  test_get(url, scheme, "https");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "aab.example");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
