#include "../include/url.h"
#include "helpers.h"

int
main() {
  // Leading and trailing C0 control or space is removed. Delete is not a C0
  // control, and so is percent-encoded like any other unsafe byte rather than
  // trimmed along with the space and control that follow it.
  test_parse(url, "\x01" "\x20" "\x1f" "https://example.com/foo" "\x7f" "\x20" "\x01", NULL);

  test_get(url, href, "https://example.com/foo%7F");
  test_get(url, scheme, "https");
  test_get(url, host, "example.com");
  test_get(url, path, "/foo%7F");

  url_destroy(&url);
}
