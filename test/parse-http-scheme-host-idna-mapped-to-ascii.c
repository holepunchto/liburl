#include "../include/url.h"
#include "helpers.h"

int
main () {
  // A code point may map to one that is ASCII, in which case the label needs no
  // Punycode encoding at all.
  test_parse(url, "https://ＡＢ.com", NULL);

  test_get(url, host, "ab.com");
  test_get(url, href, "https://ab.com/");

  url_destroy(&url);
}
