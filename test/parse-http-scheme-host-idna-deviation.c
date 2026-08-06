#include "../include/url.h"
#include "helpers.h"

int
main () {
  // Nontransitional processing leaves the deviation code point "ß" (U+00DF)
  // unchanged rather than mapping it to "ss".
  test_parse(url, "https://faß.de", NULL);

  test_get(url, host, "xn--fa-hia.de");
  test_get(url, href, "https://xn--fa-hia.de/");

  url_destroy(&url);
}
