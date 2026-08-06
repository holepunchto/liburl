#include "../include/url.h"
#include "helpers.h"

int
main() {
  // The domain is normalized to Unicode Normalization Form C, composing the
  // "A" and the combining grave accent (U+0300) into "à" (U+00E0).
  test_parse(url, "https://A\xcc\x80.com", NULL);

  test_get(url, host, "xn--0ca.com");
  test_get(url, href, "https://xn--0ca.com/");

  url_destroy(&url);
}
