#include "helpers.h"

int
main () {
  test_parse(url, "file://", NULL);

  test_component(url, href, "file:///");
  test_component(url, scheme, "file");
  test_component(url, path, "/");
}
