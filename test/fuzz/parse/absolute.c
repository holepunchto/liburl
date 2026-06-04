#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../../../include/url.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  url_t url;
  url_init(&url);

  int err = url_parse(&url, (const utf8_t *) data, size, NULL);

  if (err == 0) {
    // Re-parsing the canonical serialization must yield the same result.
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

  return 0;
}
