#include "../include/url.h"
#include "helpers.h"

int
main () {
  // ASCII letters are case folded (FOO -> foo) but the non-ASCII uppercase
  // "Ä" (U+00C4) is left unchanged; full UTS #46 folding would map it to "ä".
  test_parse(url, "https://FOOÄ.com", NULL);

  test_get(url, href, "https://fooÄ.com/");
  test_get(url, scheme, "https");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "fooÄ.com");
  test_get(url, port, "");
  test_get(url, path, "/");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
