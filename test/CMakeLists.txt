list(APPEND tests
  parse-custom-scheme-fragment
  parse-custom-scheme-query
  parse-custom-scheme-query-fragment
  parse-custom-scheme-root-path
  parse-custom-scheme-root-path-fragment
  parse-custom-scheme-root-path-query
  parse-custom-scheme-root-path-query-fragment
  parse-file-scheme
  parse-file-scheme-empty-path
  parse-file-scheme-empty-path-fragment
  parse-file-scheme-fragment
  parse-file-scheme-host
  parse-file-scheme-query
  parse-file-scheme-query-fragment
  parse-file-scheme-relative-path
  parse-file-scheme-root-path
  parse-http-scheme
  parse-http-scheme-port
  parse-http-scheme-port-default
  parse-http-scheme-relative-path
  parse-http-scheme-username
  parse-http-scheme-username-password
)

foreach(test IN LISTS tests)
  add_executable(${test} ${test}.c)

  target_link_libraries(
    ${test}
    PRIVATE
      url_static
  )

  target_include_directories(
    ${test}
    PRIVATE
      $<TARGET_PROPERTY:url,INTERFACE_INCLUDE_DIRECTORIES>
  )

  add_test(
    NAME ${test}
    COMMAND ${test}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )

  set_tests_properties(
    ${test}
    PROPERTIES
    TIMEOUT 30
  )
endforeach()
