#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(base, "http://example.com/foo/bar", NULL);
  test_parse(url, "//@@H", &base);

  utf8_string_view_t href = url_get_href(&url);

  url_t roundtrip;
  url_init(&roundtrip);
  assert(url_parse(&roundtrip, href.data, href.len, NULL) == 0);

  assert(utf8_string_view_compare(url_get_href(&roundtrip), href) == 0);

  url_destroy(&roundtrip);
  url_destroy(&url);
  url_destroy(&base);
}
