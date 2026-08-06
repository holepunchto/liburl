#include "../include/url.h"
#include "helpers.h"

int
main() {
  // A number long enough to wrap a 64 bit accumulator must not come back down
  // within the range of an address, which this one wraps around to.
  test_parse_failure(
    "http://11111111111111111111111151111111111000000000000000000000"
    "000000000000000000000000000000000000000000000004099276403/foo/bar",
    NULL
  );
}
