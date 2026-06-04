#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "http://example.com/foo/bar", NULL);

  url_t url;
  url_init(&url);
  assert(url_parse(&url, (utf8_t *) "//%3f", -1, &base) != 0);

  url_destroy(&url);
  url_destroy(&base);
}
