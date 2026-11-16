#include "helpers.h"
#include <url.h>

int
main() {
  test_parse(base, "scheme://host", NULL);

  test_parse(url, "#foo", &base);

  test_get(url, href, "scheme://host#foo");
  test_get(url, scheme, "scheme");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "host");
  test_get(url, port, "");
  test_get(url, path, "");
  test_get(url, query, "");
  test_get(url, fragment, "foo");

  url_destroy(&url);
}
