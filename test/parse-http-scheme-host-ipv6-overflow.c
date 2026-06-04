#include "../include/url.h"
#include "helpers.h"

int
main () {
  test_parse(valid, "http://[1:2:3:4:5:6:7::]/", NULL);

  test_get(valid, href, "http://[1:2:3:4:5:6:7:0]/");

  url_destroy(&valid);

  url_t overflow;
  url_init(&overflow);
  assert(url_parse(&overflow, (utf8_t *) "http://[1:2:3:4:5:6:7::4]/", -1, NULL) != 0);

  url_destroy(&overflow);
}
