#include "../include/url.h"
#include "helpers.h"

int
main() {
  // A leading zero makes the last label an octal number, and a lone zero behind it
  // is read as zero rather than rejected.
  test_parse(url, "http://00/foo/bar", NULL);

  test_get(url, href, "http://0.0.0.0/foo/bar");
  test_get(url, scheme, "http");
  test_get(url, username, "");
  test_get(url, password, "");
  test_get(url, host, "0.0.0.0");
  test_get(url, port, "");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "");
  test_get(url, fragment, "");

  url_destroy(&url);
}
