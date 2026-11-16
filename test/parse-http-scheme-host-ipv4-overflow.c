#include "helpers.h"
#include <url.h>

int
main() {
  // One past the largest address is no address at all.
  test_parse_failure("http://4294967296/foo/bar", NULL);
}
