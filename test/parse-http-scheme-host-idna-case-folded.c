#include "../include/url.h"
#include "helpers.h"

int
main () {
  // ASCII letters are folded to lowercase and so are their non-ASCII
  // counterparts, "Ä" (U+00C4) mapping to "ä" (U+00E4).
  test_parse(url, "https://FOOÄ.com", NULL);

  test_get(url, host, "xn--foo-sla.com");
  test_get(url, href, "https://xn--foo-sla.com/");

  url_destroy(&url);
}
