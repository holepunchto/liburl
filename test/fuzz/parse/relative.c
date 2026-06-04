#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../../../include/url.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  url_t base;
  url_init(&base);

  int err = url_parse(&base, (const utf8_t *) "http://example.com/foo/bar", (size_t) -1, NULL);
  assert(err == 0);

  url_t url;
  url_init(&url);

  err = url_parse(&url, (const utf8_t *) data, size, &base);

  if (err == 0) {
    // Re-parsing the canonical serialization (without a base) must yield the
    // same result, since the serialized href is always absolute.
    utf8_string_view_t href = url_get_href(&url);

    url_t roundtrip;
    url_init(&roundtrip);

    err = url_parse(&roundtrip, href.data, href.len, NULL);

    assert(err == 0);

    utf8_string_view_t reparsed = url_get_href(&roundtrip);

    assert(reparsed.len == href.len);
    assert(memcmp(reparsed.data, href.data, href.len) == 0);

    url_destroy(&roundtrip);
  }

  url_destroy(&url);
  url_destroy(&base);

  return 0;
}
