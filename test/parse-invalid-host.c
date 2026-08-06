#include "../include/url.h"
#include "helpers.h"

int
main () {
  // A host must not be empty if a port follows it, whether or not the URL is a
  // special one.
  test_parse_failure("http://:8080/foo", NULL);
  test_parse_failure("custom://:8080/foo", NULL);

  // A host must not be empty if a userinfo precedes it, again whether or not the
  // URL is a special one.
  test_parse_failure("http://user@/foo", NULL);
  test_parse_failure("custom://user@/foo", NULL);

  // The host of a special URL must not be empty at all. Note that this takes a
  // delimiter to run up against, as the slashes that would otherwise stand
  // between the scheme and the host are skipped however many of them there are.
  test_parse_failure("http://", NULL);
  test_parse_failure("http://?foo", NULL);
  test_parse_failure("http://#foo", NULL);
}
