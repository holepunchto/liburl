#include "../include/url.h"
#include "helpers.h"

int
main() {
  // Only two hexadecimal digits make an escape, so this one is left as the percent
  // sign and the two code points that it is, and a percent sign is a forbidden
  // domain code point.
  test_parse_failure("https://a%3zb.example/foo/bar", NULL);
}
