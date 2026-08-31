#include "../include/url.h"
#include "helpers.h"

int
main() {
  test_parse(url, "ht\ttps://exa\nmple.com/fo\ro/bar?ba\tz#qu\nux", NULL);

  test_get(url, href, "https://example.com/foo/bar?baz#quux");
  test_get(url, scheme, "https");
  test_get(url, host, "example.com");
  test_get(url, path, "/foo/bar");
  test_get(url, query, "baz");
  test_get(url, fragment, "quux");

  url_destroy(&url);
}
