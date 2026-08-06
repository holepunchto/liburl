#ifndef URL_IDNA_H
#define URL_IDNA_H

#include <punycode.h>
#include <stdbool.h>
#include <string.h>
#include <utf.h>
#include <utf/string.h>

#include "infra.h"
#include "unicode.h"

/**
 * The prefix that marks a label as being Punycode encoded.
 *
 * https://www.rfc-editor.org/rfc/rfc3490#section-5
 */
#define URL__IDNA_ACE_PREFIX "xn--"

#define URL__IDNA_ACE_PREFIX_LEN 4

static inline bool
url__idna_is_ascii (const utf32_t *code_points, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (code_points[i] > 0x7f) return false;
  }

  return true;
}

static inline bool
url__idna_has_ace_prefix (const utf32_t *label, size_t len) {
  return len >= URL__IDNA_ACE_PREFIX_LEN && label[0] == 'x' && label[1] == 'n' && label[2] == '-' && label[3] == '-';
}

/**
 * Checks a label against the ContextJ rules, which restrict the contexts in
 * which the zero width non-joiner and joiner may appear.
 *
 * https://www.rfc-editor.org/rfc/rfc5892#appendix-A.1
 * https://www.rfc-editor.org/rfc/rfc5892#appendix-A.2
 */
static inline bool
url__idna_check_joiners (const utf32_t *label, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (label[i] != 0x200c && label[i] != 0x200d) continue;

    // Either joiner may follow a virama, where it controls whether the two
    // characters it sits between take a conjunct form.
    if (i > 0 && url__unicode_is_virama(label[i - 1])) continue;

    // The zero width joiner has no other context in which it is allowed.
    if (label[i] == 0x200d) return false;

    // The zero width non-joiner may also keep apart two characters that would
    // otherwise join, which is the case if it is preceded by a left or dual
    // joining character and followed by a right or dual joining character,
    // disregarding any transparent characters in between.
    size_t before = i;

    while (before > 0 && url__unicode_joining_type(label[before - 1]) == url__unicode_joining_type_transparent) {
      before--;
    }

    if (before == 0) return false;

    url__unicode_joining_type_t type = url__unicode_joining_type(label[before - 1]);

    if (type != url__unicode_joining_type_left && type != url__unicode_joining_type_dual) return false;

    size_t after = i + 1;

    while (after < len && url__unicode_joining_type(label[after]) == url__unicode_joining_type_transparent) {
      after++;
    }

    if (after == len) return false;

    type = url__unicode_joining_type(label[after]);

    if (type != url__unicode_joining_type_right && type != url__unicode_joining_type_dual) return false;
  }

  return true;
}

/**
 * Checks a label of a Bidi domain name against the six numbered conditions of
 * the Bidi rule.
 *
 * https://www.rfc-editor.org/rfc/rfc5893#section-2
 */
static inline bool
url__idna_check_bidi (const utf32_t *label, size_t len) {
  if (len == 0) return true;

  bool rtl;

  // 1. The first character must be a character with Bidi property L, R, or AL.
  //    If it has the R or AL property, it is an RTL label; if it has the L
  //    property, it is an LTR label.
  switch (url__unicode_bidi_class(label[0])) {
  case url__unicode_bidi_class_r:
  case url__unicode_bidi_class_al:
    rtl = true;
    break;

  case url__unicode_bidi_class_l:
    rtl = false;
    break;

  default:
    return false;
  }

  bool european_number = false, arabic_number = false;

  // The last character that is not a nonspacing mark, those being disregarded
  // by the conditions on the end of the label.
  size_t end = 0;

  for (size_t i = 0; i < len; i++) {
    url__unicode_bidi_class_t bidi_class = url__unicode_bidi_class(label[i]);

    if (rtl) {
      // 2. In an RTL label, only characters with the Bidi properties R, AL, AN,
      //    EN, ES, CS, ET, ON, BN, or NSM are allowed.
      switch (bidi_class) {
      case url__unicode_bidi_class_r:
      case url__unicode_bidi_class_al:
      case url__unicode_bidi_class_an:
      case url__unicode_bidi_class_en:
      case url__unicode_bidi_class_es:
      case url__unicode_bidi_class_cs:
      case url__unicode_bidi_class_et:
      case url__unicode_bidi_class_on:
      case url__unicode_bidi_class_bn:
      case url__unicode_bidi_class_nsm:
        break;

      default:
        return false;
      }
    } else {
      // 5. In an LTR label, only characters with the Bidi properties L, EN, ES,
      //    CS, ET, ON, BN, or NSM are allowed.
      switch (bidi_class) {
      case url__unicode_bidi_class_l:
      case url__unicode_bidi_class_en:
      case url__unicode_bidi_class_es:
      case url__unicode_bidi_class_cs:
      case url__unicode_bidi_class_et:
      case url__unicode_bidi_class_on:
      case url__unicode_bidi_class_bn:
      case url__unicode_bidi_class_nsm:
        break;

      default:
        return false;
      }
    }

    if (bidi_class == url__unicode_bidi_class_en) european_number = true;
    else if (bidi_class == url__unicode_bidi_class_an) arabic_number = true;

    if (bidi_class != url__unicode_bidi_class_nsm) end = i;
  }

  if (rtl) {
    // 3. In an RTL label, the end of the label must be a character with Bidi
    //    property R, AL, EN, or AN, followed by zero or more characters with
    //    Bidi property NSM.
    switch (url__unicode_bidi_class(label[end])) {
    case url__unicode_bidi_class_r:
    case url__unicode_bidi_class_al:
    case url__unicode_bidi_class_en:
    case url__unicode_bidi_class_an:
      break;

    default:
      return false;
    }

    // 4. In an RTL label, if an EN is present, no AN may be present, and vice
    //    versa.
    if (european_number && arabic_number) return false;
  } else {
    // 6. In an LTR label, the end of the label must be a character with Bidi
    //    property L or EN, followed by zero or more characters with Bidi
    //    property NSM.
    switch (url__unicode_bidi_class(label[end])) {
    case url__unicode_bidi_class_l:
    case url__unicode_bidi_class_en:
      break;

    default:
      return false;
    }
  }

  return true;
}

/**
 * Checks a label against the validity criteria, apart from the two that the
 * conversion of the domain checks itself.
 *
 * https://www.unicode.org/reports/tr46/#Validity_Criteria
 */
static inline bool
url__idna_is_valid_label (const utf32_t *label, size_t len) {
  // Only a non-empty label is subject to the validity criteria.
  if (len == 0) return true;

  // 1. The label must be in Unicode Normalization Form NFC. Only a label that
  //    was converted from Punycode can fail this, the domain having been
  //    normalized as a whole before being broken into labels, and so it is
  //    checked as part of that conversion.

  // 2. and 3. are skipped as CheckHyphens is false.

  // 4. If not CheckHyphens, the label must not begin with "xn--".
  if (url__idna_has_ace_prefix(label, len)) return false;

  // 5. The label must not contain a U+002E ( . ) FULL STOP. The domain is
  //    broken into labels at every full stop, so this holds by construction.

  // 6. The label must not begin with a combining mark, that is:
  //    General_Category=Mark.
  if (url__unicode_is_mark(label[0])) return false;

  // 7. Each code point in the label must only have certain status values
  //    according to the IDNA mapping table, which for nontransitional
  //    processing means either valid or deviation.
  for (size_t i = 0; i < len; i++) {
    const utf32_t *mapping;
    size_t mapping_len;

    if (url__idna_status(label[i], &mapping, &mapping_len) != url__idna_status_valid) return false;
  }

  // 8. If CheckJoiners, the label must satisfy the ContextJ rules.
  if (!url__idna_check_joiners(label, len)) return false;

  // 9. The Bidi rule is checked once the domain as a whole is known to be a Bidi
  //    domain name.

  return true;
}

/**
 * Converts a domain to its ASCII form.
 *
 * This is Unicode ToASCII with the flags that the domain parser needs, being
 * CheckBidi and CheckJoiners set to true and UseSTD3ASCIIRules, CheckHyphens,
 * VerifyDnsLength, Transitional_Processing, and IgnoreInvalidPunycode all set
 * to false. As a domain for which an error is recorded is of no use to the
 * domain parser, such a domain is rejected by returning -1 rather than
 * converted as far as is possible.
 *
 * https://www.unicode.org/reports/tr46/#ToASCII
 * https://url.spec.whatwg.org/#concept-domain-to-ascii
 */
static inline int
url__idna_to_ascii (utf8_string_view_t input, utf8_string_t *result) {
  int err;

  // The domain handed to the domain parser is the result of a UTF-8 decode,
  // which replaces every ill-formed byte sequence with U+FFFD. As U+FFFD is
  // disallowed, and so fails the validity criteria, such a domain may be
  // rejected without decoding it.
  if (!utf8_validate(input.data, input.len)) return -1;

  utf32_string_t decoded, normalized, converted, label;

  utf32_string_init(&decoded);
  utf32_string_init(&normalized);
  utf32_string_init(&converted);
  utf32_string_init(&label);

  bool bidi = false;

  err = utf32_string_reserve(&decoded, utf32_length_from_utf8(input.data, input.len));
  if (err < 0) goto err;

  decoded.len = utf8_convert_to_utf32(input.data, input.len, decoded.data);

  // 1. Map each code point according to its status in the IDNA mapping table and
  //    2. normalize the domain to Unicode Normalization Form C, decomposing as
  //    the mapping goes so that the two steps take a single pass.
  for (size_t i = 0; i < decoded.len; i++) {
    const utf32_t *mapping;
    size_t mapping_len;

    switch (url__idna_status(decoded.data[i], &mapping, &mapping_len)) {
    case url__idna_status_mapped:
      for (size_t j = 0; j < mapping_len; j++) {
        err = url__unicode_decompose(mapping[j], &normalized);
        if (err < 0) goto err;
      }
      break;

    case url__idna_status_ignored:
      break;

    // A disallowed code point is left in place for the validity criteria to
    // reject once the domain has been mapped and normalized.
    case url__idna_status_valid:
    case url__idna_status_disallowed:
    default:
      err = url__unicode_decompose(decoded.data[i], &normalized);
      if (err < 0) goto err;
      break;
    }
  }

  normalized.len = url__unicode_recompose(normalized.data, normalized.len);

  // 3. Break the domain into labels at U+002E ( . ) FULL STOP, 4. convert every
  //    label that is Punycode encoded back to Unicode, and verify that the label
  //    meets the validity criteria.
  for (size_t i = 0, start = 0; i <= normalized.len; i++) {
    if (i != normalized.len && normalized.data[i] != '.') continue;

    if (start > 0) {
      err = utf32_string_append_character(&converted, '.');
      if (err < 0) goto err;
    }

    size_t offset = converted.len;

    if (url__idna_has_ace_prefix(&normalized.data[start], i - start)) {
      // A Punycode encoded label may not contain a non-ASCII code point.
      if (!url__idna_is_ascii(&normalized.data[start], i - start)) goto err;

      size_t encoded_len = i - start - URL__IDNA_ACE_PREFIX_LEN;

      utf32_string_clear(&label);

      // The decoder needs room for the most that the label could decode to, being
      // as many code points as it was encoded from. That is more than it usually
      // needs, so a label short enough to be decoded within this buffer is, which
      // keeps the buffer above off the heap for as long as what comes out of the
      // decoder fits in it. A domain name label runs to 63 code points at most,
      // less the prefix that has already been taken off.
      utf32_t buf[64];

      utf32_t *decoded_label = buf;

      if (encoded_len > sizeof(buf) / sizeof(utf32_t)) {
        err = utf32_string_reserve(&label, utf32_max_length_from_punycode(encoded_len));
        if (err < 0) goto err;

        decoded_label = label.data;
      }

      size_t decoded_len;

      // The basic code points of the label are handed to the decoder as they
      // already stand, rather than narrowed to bytes first.
      err = punycode_decode_utf32(
        &normalized.data[start + URL__IDNA_ACE_PREFIX_LEN],
        encoded_len,
        decoded_label,
        &decoded_len
      );
      if (err < 0) goto err;

      if (decoded_label == buf) {
        // Now that the length is known, only that much of the buffer above is
        // taken, which is what keeps a label of a usual length within it.
        err = utf32_string_append_literal(&label, buf, decoded_len);
        if (err < 0) goto err;
      } else {
        label.len = decoded_len;
      }

      // A label that decodes to nothing, or to nothing but ASCII, had no
      // business being Punycode encoded to begin with.
      if (label.len == 0 || url__idna_is_ascii(label.data, label.len)) goto err;

      // The first of the validity criteria: the label must be in Normalization
      // Form C. Normalizing it is at the same time how it joins the converted
      // domain, the two being one and the same if it was already normalized.
      err = url__unicode_normalize(label.data, label.len, &converted);
      if (err < 0) goto err;

      if (converted.len - offset != label.len) goto err;
      if (memcmp(&converted.data[offset], label.data, label.len * sizeof(utf32_t)) != 0) goto err;
    } else {
      err = utf32_string_append_literal(&converted, &normalized.data[start], i - start);
      if (err < 0) goto err;
    }

    if (!url__idna_is_valid_label(&converted.data[offset], converted.len - offset)) goto err;

    start = i + 1;
  }

  // A Bidi domain name is a domain name containing at least one character with
  // Bidi_Class R, AL, or AN.
  for (size_t i = 0; i < converted.len && !bidi; i++) {
    if (converted.data[i] < URL__UNICODE_FIRST_RTL) continue;

    switch (url__unicode_bidi_class(converted.data[i])) {
    case url__unicode_bidi_class_r:
    case url__unicode_bidi_class_al:
    case url__unicode_bidi_class_an:
      bidi = true;
      break;

    default:
      break;
    }
  }

  // The last of the validity criteria, which only the labels of a Bidi domain
  // name are subject to.
  if (bidi) {
    for (size_t i = 0, start = 0; i <= converted.len; i++) {
      if (i != converted.len && converted.data[i] != '.') continue;

      if (!url__idna_check_bidi(&converted.data[start], i - start)) goto err;

      start = i + 1;
    }
  }

  // ToASCII step 3: convert every label with non-ASCII characters into Punycode
  // and prefix it by "xn--". Step 4, verifying the DNS length restrictions, is
  // skipped as VerifyDnsLength is false.
  for (size_t i = 0, start = 0; i <= converted.len; i++) {
    if (i != converted.len && converted.data[i] != '.') continue;

    if (start > 0) {
      err = utf8_string_append_character(result, '.');
      if (err < 0) goto err;
    }

    if (url__idna_is_ascii(&converted.data[start], i - start)) {
      for (size_t j = start; j < i; j++) {
        err = utf8_string_append_character(result, (utf8_t) converted.data[j]);
        if (err < 0) goto err;
      }
    } else {
      err = utf8_string_append_literal(result, (utf8_t *) URL__IDNA_ACE_PREFIX, URL__IDNA_ACE_PREFIX_LEN);
      if (err < 0) goto err;

      size_t bound = punycode_max_length_from_utf32(i - start);

      if (bound == (size_t) -1 || bound > SIZE_MAX - result->len) goto err;

      // The label is encoded straight into the result, which is grown to the most
      // that encoding it could call for beforehand.
      err = utf8_string_reserve(result, result->len + bound);
      if (err < 0) goto err;

      size_t encoded_len;

      err = punycode_encode_utf8(&converted.data[start], i - start, &result->data[result->len], &encoded_len);
      if (err < 0) goto err;

      result->len += encoded_len;
    }

    start = i + 1;
  }

  err = 0;

  goto done;

err:
  err = -1;

done:
  utf32_string_destroy(&decoded);
  utf32_string_destroy(&normalized);
  utf32_string_destroy(&converted);
  utf32_string_destroy(&label);

  return err;
}

#endif // URL_IDNA_H
