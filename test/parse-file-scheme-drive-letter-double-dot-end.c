#include "../include/url.h"
#include "helpers.h"

int
main () {
  // A double dot segment may not shorten the path past a drive letter, which is
  // the only segment there is to shorten here.
  test_parse(url, "file://c:/..", NULL);

  test_get(url, href, "file:///c:/");
  test_get(url, scheme, "file");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "");
  test_get(url, port, "");
  test_get(url, path, "/c:/");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
