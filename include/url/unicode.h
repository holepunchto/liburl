#ifndef URL_UNICODE_H
#define URL_UNICODE_H

#include <normalize.h>
#include <stdbool.h>
#include <stdint.h>
#include <url/unicode-tables.h>
#include <utf.h>

/**
 * The first code point with Bidi_Class R, AL, or AN, counting the classes that
 * unassigned code points default to.
 */
#define URL__UNICODE_FIRST_RTL 0x590

/**
 * Looks up the value that a sorted, gap-free list of ranges gives a code point,
 * which is the value of the last range that starts at or below it. The search is
 * narrowed down to the block that the code point belongs to.
 */
static inline uint8_t
url__unicode_range_value (const url_unicode_range_t *ranges, const uint16_t *blocks, utf32_t c) {
  size_t block = c >> URL_UNICODE_BLOCK_SHIFT;

  size_t lo = blocks[block], hi = blocks[block + 1];

  while (lo < hi) {
    size_t mid = lo + (hi - lo + 1) / 2;

    if (ranges[mid] >> 8 <= c) lo = mid;
    else hi = mid - 1;
  }

  return ranges[lo] & 0xff;
}

static inline uint8_t
url__unicode_properties (utf32_t c) {
  return url__unicode_range_value(url__unicode_property_ranges, url__unicode_property_blocks, c);
}

// https://www.unicode.org/reports/tr44/#Bidi_Class
static inline url__unicode_bidi_class_t
url__unicode_bidi_class (utf32_t c) {
  return (url__unicode_bidi_class_t) (url__unicode_properties(c) & 0xf);
}

// https://www.unicode.org/reports/tr44/#Joining_Type
static inline url__unicode_joining_type_t
url__unicode_joining_type (utf32_t c) {
  return (url__unicode_joining_type_t) ((url__unicode_properties(c) >> 4) & 0x7);
}

// https://www.unicode.org/reports/tr44/#General_Category_Values
static inline bool
url__unicode_is_mark (utf32_t c) {
  // A combining mark is the first thing to have a combining class, so nothing
  // below the first of them can be one.
  if (c < NORMALIZE_FIRST_MARK) return false;

  return (url__unicode_properties(c) >> 7) != 0;
}

/**
 * Looks up the IDNA status of a code point, setting `mapping` and `len` to its
 * mapping if the status is `url__idna_status_mapped`.
 *
 * https://www.unicode.org/reports/tr46/#IDNA_Mapping_Table
 */
static inline url__idna_status_t
url__idna_status (utf32_t c, const utf32_t **mapping, size_t *len) {
  uint32_t value;

  if (c < 0x80) {
    value = url__idna_ascii[c];
  } else {
    size_t block = c >> URL_UNICODE_BLOCK_SHIFT;

    size_t lo = url__idna_blocks[block], hi = url__idna_blocks[block + 1];

    while (lo < hi) {
      size_t mid = lo + (hi - lo + 1) / 2;

      if (url__idna_ranges[mid].start <= c) lo = mid;
      else hi = mid - 1;
    }

    value = url__idna_ranges[lo].mapping;
  }

  *mapping = &url__idna_mapping_data[value >> 7];
  *len = (value >> 2) & 0x1f;

  return (url__idna_status_t) (value & 0x3);
}

#endif // URL_UNICODE_H
