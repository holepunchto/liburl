#include "../include/url.h"
#include "helpers.h"

int
main() {
  // Only a delimiter may follow the digits of a port.
  test_parse_failure("http://example.com:8080x/foo", NULL);
  test_parse_failure("http://example.com:80:80/foo", NULL);
  test_parse_failure("http://example.com:-1/foo", NULL);
  test_parse_failure("custom://example.com:8080x/foo", NULL);

  // A port must fit in sixteen bits.
  test_parse_failure("http://example.com:65536/foo", NULL);
  test_parse_failure("http://example.com:99999999999999999999/foo", NULL);
}
