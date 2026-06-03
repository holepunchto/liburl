list(APPEND fuzzers
  parse/absolute
  parse/relative
)

foreach(fuzzer IN LISTS fuzzers)
  string(REPLACE "/" "_" name ${fuzzer})

  add_fuzzer(fuzz_${name} ${fuzzer}.c)

  target_link_libraries(
    fuzz_${name}
    PRIVATE
      url_static
  )
endforeach()
