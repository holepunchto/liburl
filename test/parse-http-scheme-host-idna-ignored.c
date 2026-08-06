#include "../include/url.h"
#include "helpers.h"

int
main() {
  // The soft hyphen (U+00AD) is ignored, being dropped from the domain.
  test_parse(url, "https://ex­ample.com", NULL);

  test_get(url, host, "example.com");
  test_get(url, href, "https://example.com/");

  url_destroy(&url);
}
