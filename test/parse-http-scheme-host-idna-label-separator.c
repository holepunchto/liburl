#include "../include/url.h"
#include "helpers.h"

int
main () {
  // The ideographic full stop (U+3002) maps to U+002E FULL STOP and so ends up
  // separating two labels.
  test_parse(url, "https://ex。ample.com", NULL);

  test_get(url, host, "ex.ample.com");
  test_get(url, href, "https://ex.ample.com/");

  url_destroy(&url);
}
