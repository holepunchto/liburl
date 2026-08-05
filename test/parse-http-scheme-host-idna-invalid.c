#include "../include/url.h"
#include "helpers.h"

int
main () {
  // A label may not begin with a combining mark, here the combining grave
  // accent (U+0300).
  test_parse_failure("https://\xcc\x80.com", NULL);

  // A domain that maps to nothing at all, the soft hyphen (U+00AD) being
  // ignored, is no domain.
  test_parse_failure("https://\xc2\xad", NULL);

  // A disallowed code point, here U+FFFD REPLACEMENT CHARACTER, may not appear
  // in a label.
  test_parse_failure("https://a\xef\xbf\xbd" "b.com", NULL);

  // A Punycode encoded label may not contain a non-ASCII code point.
  test_parse_failure("https://xn--\xc3\xa0.com", NULL);

  // The zero width non-joiner (U+200C) may only appear after a virama or
  // between two joining characters.
  test_parse_failure("https://a\xe2\x80\x8c" "b.com", NULL);

  // Every label of a Bidi domain name, being one that holds a right-to-left
  // character, must begin with a character that is either left-to-right or
  // right-to-left, which the European number "0" is not.
  test_parse_failure("https://0\xc3\xa0.\xd7\x90", NULL);
}
