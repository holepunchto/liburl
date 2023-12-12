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
  parse-file-scheme-empty-path
  parse-file-scheme-empty-path-fragment
  parse-file-scheme-fragment
  parse-file-scheme-host
  parse-file-scheme-query
  parse-file-scheme-query-fragment
  parse-file-scheme-relative-path
  parse-file-scheme-relative-path-with-scheme
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
  set-scheme-http-to-ftp
  set-scheme-http-to-https
  set-scheme-http-to-https-default-port
  set-username
  set-username-longer
  set-username-longer-with-password
  set-username-shorter
  set-username-shorter-with-password
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
