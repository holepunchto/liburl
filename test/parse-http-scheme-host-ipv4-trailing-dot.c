#include "../include/url.h"
#include "helpers.h"

int
main() {
  // A trailing full stop leaves an empty last part, which is dropped, so the part
  // before it is the one that takes the bits left over.
  test_parse(url, "http://8.8./foo/bar", NULL);

  test_get(url, href, "http://8.0.0.8/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "8.0.0.8");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
