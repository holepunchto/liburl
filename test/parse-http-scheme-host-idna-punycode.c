#include "../include/url.h"
#include "helpers.h"

int
main() {
  // A domain that is already Punycode encoded is left as it is, the encoding
  // round-tripping through the decoding that validates it.
  test_parse(url, "https://xn--fa-hia.de", NULL);

  test_get(url, host, "xn--fa-hia.de");
  test_get(url, href, "https://xn--fa-hia.de/");

  url_destroy(&url);
}
