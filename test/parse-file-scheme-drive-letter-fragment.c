#include "../include/url.h"
#include "helpers.h"

int
main () {
  // A drive letter in the host position becomes the first segment of the path,
  // with the fragment that follows it left where it is.
  test_parse(url, "file://c:#quux", NULL);

  test_get(url, href, "file:///c:#quux");
  test_get(url, scheme, "file");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "");
  test_get(url, port, "");
  test_get(url, path, "/c:");
  test_get(url, query, "");
  test_get(url, fragment, "quux");

  url_destroy(&url);
}
