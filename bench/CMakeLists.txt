list(APPEND benches
  parse
)

foreach(bench IN LISTS benches)
  add_executable(${bench} ${bench}.c)

  target_link_libraries(
    ${bench}
    PRIVATE
      url_static
  )
endforeach()
