#include "../include/url.h"
#include "helpers.h"

int
main () {
  // An ASCII domain is lowercased and left at that, for web compatibility, even
  // though xn--8i7caa decodes to "ｗｗｗ" whose code points are mapped
  // and would therefore fail the validity criteria.
  test_parse(url, "https://xn--8i7caa.com", NULL);

  test_get(url, host, "xn--8i7caa.com");
  test_get(url, href, "https://xn--8i7caa.com/");

  url_destroy(&url);
}
