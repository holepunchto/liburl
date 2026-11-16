#include "helpers.h"
#include <url.h>

int
main() {
  test_parse(url, "scheme:/", NULL);

  test_get(url, href, "scheme:/");
  test_get(url, scheme, "scheme");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "");
  test_get(url, port, "");
  test_get(url, path, "/");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
