#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(url, "https://日本語.jp", NULL);

  test_get(url, href, "https://xn--wgv71a119e.jp/");
  test_get(url, scheme, "https");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "xn--wgv71a119e.jp");
  test_get(url, port, "");
  test_get(url, path, "/");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
