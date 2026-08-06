#include "../include/url.h"
#include "helpers.h"

int
main() {
  // A drive letter in the host position is no host at all, but the first segment
  // of the path, even when nothing follows it.
  test_parse(url, "file://c:", NULL);

  test_get(url, href, "file:///c:");
  test_get(url, scheme, "file");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "");
  test_get(url, port, "");
  test_get(url, path, "/c:");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
