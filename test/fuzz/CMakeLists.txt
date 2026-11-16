add_fuzz_executables(
  parse/absolute.c
  parse/relative.c
  PREFIX fuzz_
  DURATION 600
  LINK
    url::static
)
