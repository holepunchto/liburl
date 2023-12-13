list(APPEND tests
  parse-custom-scheme-fragment
  parse-custom-scheme-query
  parse-custom-scheme-query-fragment
  parse-custom-scheme-relative-bare-path
  parse-custom-scheme-relative-bare-path-username
  parse-custom-scheme-relative-opaque-path-fragment
  parse-custom-scheme-relative-opaque-path-fragment-with-query
  parse-custom-scheme-relative-path
  parse-custom-scheme-root-path
  parse-custom-scheme-root-path-fragment
  parse-custom-scheme-root-path-query
  parse-custom-scheme-root-path-query-fragment
  parse-file-scheme
  parse-file-scheme-double-dot
  parse-file-scheme-double-dot-drive-letter
  parse-file-scheme-double-dot-end
  parse-file-scheme-double-dot-middle
  parse-file-scheme-double-dot-root-path
  parse-file-scheme-drive-letter
  parse-file-scheme-drive-letter-normalize
  parse-file-scheme-empty-path
  parse-file-scheme-empty-path-fragment
  parse-file-scheme-fragment
  parse-file-scheme-host
  parse-file-scheme-query
  parse-file-scheme-query-fragment
  parse-file-scheme-relative-path
  parse-file-scheme-relative-path-with-scheme
  parse-file-scheme-relative-path-with-scheme-drive-letter
  parse-file-scheme-relative-path-with-scheme-drive-letter-in-base
  parse-file-scheme-root-path
  parse-http-scheme
  parse-http-scheme-base
  parse-http-scheme-port
  parse-http-scheme-port-default
  parse-http-scheme-protocol-relative
  parse-http-scheme-query
  parse-http-scheme-query-percent-encode
  parse-http-scheme-relative-bare-path
  parse-http-scheme-relative-path
  parse-http-scheme-relative-path-with-scheme
  parse-http-scheme-username
  parse-http-scheme-username-password
  parse-http-scheme-username-password-percent-encode
  parse-http-scheme-username-percent-encode
)

foreach(test IN LISTS tests)
  add_executable(${test} ${test}.c)

  target_link_libraries(
    ${test}
    PRIVATE
      url_static
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
