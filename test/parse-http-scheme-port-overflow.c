#include "../include/url.h"
#include "helpers.h"

int
main() {
  test_parse(valid, "http://example.com:65535/", NULL);

  test_get(valid, port, "65535");

  url_destroy(&valid);

  url_t exceeds;
  url_init(&exceeds);
  assert(url_parse(&exceeds, (utf8_t *) "http://example.com:65536/", -1, NULL) != 0);

  url_destroy(&exceeds);

  url_t wrap;
  url_init(&wrap);
  assert(url_parse(&wrap, (utf8_t *) "http://example.com:4294967396/", -1, NULL) != 0);

  url_destroy(&wrap);
}
