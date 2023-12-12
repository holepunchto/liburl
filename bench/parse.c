#include "../include/url.h"

int
main () {
  url_t url;

  for (size_t i = 0; i < 10000000; i++) {
    url_init(&url);
    url_parse(&url, (utf8_t *) "https://example.com/hello/world?query=string#fragment", -1, NULL);
    url_destroy(&url);
  }
}
