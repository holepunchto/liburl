#ifndef URL_UNICODE_H
#define URL_UNICODE_H

#include <stdbool.h>
#include <stdint.h>
#include <url/unicode-tables.h>
#include <utf.h>
#include <utf/string.h>

/**
 * The constants of the algorithmic Hangul syllable decomposition.
 *
 * https://www.unicode.org/versions/latest/core-spec/chapter-3/#G56669
 */
enum {
  url__unicode_hangul_s_base = 0xac00,
  url__unicode_hangul_l_base = 0x1100,
  url__unicode_hangul_v_base = 0x1161,
  url__unicode_hangul_t_base = 0x11a7,
  url__unicode_hangul_l_count = 19,
  url__unicode_hangul_v_count = 21,
  url__unicode_hangul_t_count = 28,
  url__unicode_hangul_n_count = url__unicode_hangul_v_count * url__unicode_hangul_t_count,
  url__unicode_hangul_s_count = url__unicode_hangul_l_count * url__unicode_hangul_n_count,
};

/**
 * The first code point that is a combining mark, and so the first with a
 * combining class other than zero. It is also the first that takes part in a
 * composition as the second of its two code points.
 */
#define URL__UNICODE_FIRST_MARK 0x300

/**
 * The first code point with a canonical decomposition.
 */
#define URL__UNICODE_FIRST_DECOMPOSABLE 0xc0

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
  if (c < URL__UNICODE_FIRST_MARK) return false;

  return (url__unicode_properties(c) >> 7) != 0;
}

// https://www.unicode.org/reports/tr44/#Canonical_Combining_Class
static inline uint8_t
url__unicode_combining_class (utf32_t c) {
  if (c < URL__UNICODE_FIRST_MARK) return 0;

  return url__unicode_range_value(
    url__unicode_combining_class_ranges,
    url__unicode_combining_class_blocks,
    c
  );
}

// https://www.rfc-editor.org/rfc/rfc5892#appendix-A.1
static inline bool
url__unicode_is_virama (utf32_t c) {
  return url__unicode_combining_class(c) == 9;
}

/**
 * Appends the full canonical decomposition of `c` to `result`.
 *
 * https://www.unicode.org/reports/tr15/#Canonical_Decomposition
 */
static inline int
url__unicode_decompose (utf32_t c, utf32_string_t *result) {
  int err;

  if (c >= url__unicode_hangul_s_base && c < url__unicode_hangul_s_base + url__unicode_hangul_s_count) {
    uint32_t index = c - url__unicode_hangul_s_base;

    err = utf32_string_append_character(result, url__unicode_hangul_l_base + index / url__unicode_hangul_n_count);
    if (err < 0) return err;

    err = utf32_string_append_character(result, url__unicode_hangul_v_base + (index % url__unicode_hangul_n_count) / url__unicode_hangul_t_count);
    if (err < 0) return err;

    uint32_t t = index % url__unicode_hangul_t_count;

    if (t != 0) {
      err = utf32_string_append_character(result, url__unicode_hangul_t_base + t);
      if (err < 0) return err;
    }

    return 0;
  }

  if (c < URL__UNICODE_FIRST_DECOMPOSABLE) return utf32_string_append_character(result, c);

  size_t block = c >> URL_UNICODE_BLOCK_SHIFT;

  size_t lo = url__unicode_decomposition_blocks[block], hi = url__unicode_decomposition_blocks[block + 1];

  while (lo < hi) {
    size_t mid = lo + (hi - lo) / 2;

    const url__unicode_decomposition_t *entry = &url__unicode_decompositions[mid];

    if (entry->code_point < c) {
      lo = mid + 1;
    } else if (entry->code_point > c) {
      hi = mid;
    } else {
      return utf32_string_append_literal(
        result,
        &url__unicode_decomposition_data[entry->decomposition & 0xffffff],
        entry->decomposition >> 24
      );
    }
  }

  return utf32_string_append_character(result, c);
}

/**
 * Returns the primary composite of `first` and `second`, or 0 if the two do not
 * compose.
 *
 * https://www.unicode.org/reports/tr15/#Primary_Composite
 */
static inline utf32_t
url__unicode_compose (utf32_t first, utf32_t second) {
  // The second of the two code points of a composition is always a combining
  // mark, or else a Hangul vowel or trailing consonant, all of which sit above
  // the first combining mark.
  if (second < URL__UNICODE_FIRST_MARK) return 0;

  if (
    first >= url__unicode_hangul_l_base &&
    first < url__unicode_hangul_l_base + url__unicode_hangul_l_count &&
    second >= url__unicode_hangul_v_base &&
    second < url__unicode_hangul_v_base + url__unicode_hangul_v_count
  ) {
    return url__unicode_hangul_s_base +
           ((first - url__unicode_hangul_l_base) * url__unicode_hangul_v_count + (second - url__unicode_hangul_v_base)) * url__unicode_hangul_t_count;
  }

  if (
    first >= url__unicode_hangul_s_base &&
    first < url__unicode_hangul_s_base + url__unicode_hangul_s_count &&
    (first - url__unicode_hangul_s_base) % url__unicode_hangul_t_count == 0 &&
    second > url__unicode_hangul_t_base &&
    second < url__unicode_hangul_t_base + url__unicode_hangul_t_count
  ) {
    return first + (second - url__unicode_hangul_t_base);
  }

  size_t lo = 0, hi = sizeof(url__unicode_compositions) / sizeof(url__unicode_composition_t);

  while (lo < hi) {
    size_t mid = lo + (hi - lo) / 2;

    const url__unicode_composition_t *entry = &url__unicode_compositions[mid];

    if (entry->first != first) {
      if (entry->first < first) lo = mid + 1;
      else hi = mid;
    } else if (entry->second != second) {
      if (entry->second < second) lo = mid + 1;
      else hi = mid;
    } else {
      return entry->composite;
    }
  }

  return 0;
}

/**
 * Puts the canonical decomposition held in `data` into Normalization Form C in
 * place, returning its new length. This is the second half of normalizing, the
 * first being to decompose each code point with `url__unicode_decompose`.
 *
 * https://www.unicode.org/reports/tr15/#Description_Norm
 */
static inline size_t
url__unicode_recompose (utf32_t *data, size_t len) {
  if (len == 0) return 0;

  // Put the decomposition in canonical order by moving each combining mark
  // ahead of any preceding mark of a higher combining class.
  for (size_t i = 1; i < len; i++) {
    uint8_t combining_class = url__unicode_combining_class(data[i]);

    if (combining_class == 0) continue;

    for (size_t j = i; j > 0 && url__unicode_combining_class(data[j - 1]) > combining_class; j--) {
      utf32_t c = data[j - 1];

      data[j - 1] = data[j];
      data[j] = c;
    }
  }

  // Compose each code point with the last starter, unless a code point of the
  // same or a higher combining class stands between the two and blocks it.
  size_t starter = 0, out = 1;

  uint32_t last = url__unicode_combining_class(data[0]);

  // A sequence beginning with a combining mark has no starter to compose with.
  if (last != 0) last = 0xff;

  for (size_t i = 1; i < len; i++) {
    utf32_t c = data[i];

    uint8_t combining_class = url__unicode_combining_class(c);

    utf32_t composite = url__unicode_compose(data[starter], c);

    if (composite != 0 && (last == 0 || last < combining_class)) {
      data[starter] = composite;
    } else {
      if (combining_class == 0) starter = out;

      last = combining_class;

      data[out++] = c;
    }
  }

  return out;
}

/**
 * Appends `input` to `result` in Normalization Form C.
 *
 * https://www.unicode.org/reports/tr15/#Description_Norm
 */
static inline int
url__unicode_normalize (const utf32_t *input, size_t len, utf32_string_t *result) {
  int err;

  size_t start = result->len;

  for (size_t i = 0; i < len; i++) {
    err = url__unicode_decompose(input[i], result);
    if (err < 0) return err;
  }

  result->len = start + url__unicode_recompose(&result->data[start], result->len - start);

  return 0;
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
