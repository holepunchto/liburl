#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <utf.h>
#include <utf/string.h>

#include "../../include/url/idna.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  utf8_string_t ascii;
  utf8_string_init(&ascii);

  int err = url__idna_to_ascii(utf8_string_view_init((const utf8_t *) data, size), &ascii);

  if (err == 0) {
    // A converted domain is always ASCII.
    assert(ascii_validate(ascii.data, ascii.len));

    // Converting a domain is idempotent, so converting the result again must
    // leave it unchanged.
    utf8_string_t again;
    utf8_string_init(&again);

    err = url__idna_to_ascii(utf8_string_view(&ascii), &again);

    assert(err == 0);
    assert(again.len == ascii.len);
    assert(memcmp(again.data, ascii.data, ascii.len) == 0);

    utf8_string_destroy(&again);
  }

  utf8_string_destroy(&ascii);

  return 0;
}
