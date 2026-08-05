list(APPEND tests
  parse-non-special-scheme-double-slash-path-idempotence
  parse-non-special-scheme-trailing-dot-segment-idempotence
  parse-custom-scheme-fragment
  parse-custom-scheme-host-uppercase-preserved
  parse-custom-scheme-query
  parse-custom-scheme-query-fragment
  parse-custom-scheme-relative-absolute-path
  parse-custom-scheme-relative-absolute-path-empty-host
  parse-custom-scheme-relative-absolute-path-host
  parse-custom-scheme-relative-absolute-path-host-empty-path
  parse-custom-scheme-relative-absolute-path-host-no-path
  parse-custom-scheme-relative-bare-path
  parse-custom-scheme-relative-bare-path-username
  parse-custom-scheme-relative-double-dot-host
  parse-custom-scheme-relative-double-dot-host-empty-path
  parse-custom-scheme-relative-double-dot-host-no-path
  parse-custom-scheme-relative-empty-path
  parse-custom-scheme-relative-empty-path-host
  parse-custom-scheme-relative-empty-path-host-empty-path
  parse-custom-scheme-relative-empty-path-host-no-path
  parse-custom-scheme-relative-fragment-host
  parse-custom-scheme-relative-fragment-host-empty-path
  parse-custom-scheme-relative-fragment-host-no-path
  parse-custom-scheme-relative-opaque-path-fragment
  parse-custom-scheme-relative-opaque-path-fragment-with-query
  parse-custom-scheme-relative-path
  parse-custom-scheme-relative-path-empty-host
  parse-custom-scheme-relative-path-host
  parse-custom-scheme-relative-path-host-empty-path
  parse-custom-scheme-relative-path-host-no-path
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
  parse-file-scheme-drive-letter-double-dot-end
  parse-file-scheme-drive-letter-empty-path
  parse-file-scheme-drive-letter-fragment
  parse-file-scheme-drive-letter-normalize
  parse-file-scheme-drive-letter-query
  parse-file-scheme-empty-path
  parse-file-scheme-empty-path-fragment
  parse-file-scheme-fragment
  parse-file-scheme-host
  parse-file-scheme-protocol-only
  parse-file-scheme-query
  parse-file-scheme-query-fragment
  parse-file-scheme-relative-path
  parse-file-scheme-relative-path-with-scheme
  parse-file-scheme-relative-path-with-scheme-drive-letter
  parse-file-scheme-relative-path-with-scheme-drive-letter-in-base
  parse-file-scheme-relative-protocol-only
  parse-file-scheme-root-path
  parse-http-scheme
  parse-http-scheme-base
  parse-http-scheme-extra-slashes
  parse-http-scheme-host-idna
  parse-http-scheme-host-idna-ascii-preserved
  parse-http-scheme-host-idna-case-folded
  parse-http-scheme-host-idna-deviation
  parse-http-scheme-host-idna-ignored
  parse-http-scheme-host-idna-invalid
  parse-http-scheme-host-idna-label-separator
  parse-http-scheme-host-idna-mapped-to-ascii
  parse-http-scheme-host-idna-normalized
  parse-http-scheme-host-idna-punycode
  parse-http-scheme-host-ipv4
  parse-http-scheme-host-ipv6
  parse-http-scheme-host-ipv6-overflow
  parse-http-scheme-host-ipv6-port
  parse-http-scheme-host-mixed-case-path-preserved
  parse-http-scheme-host-uppercase
  parse-http-scheme-password
  parse-http-scheme-password-at-sign
  parse-http-scheme-port
  parse-http-scheme-port-default
  parse-http-scheme-port-leading-zeros
  parse-http-scheme-port-leading-zeros-default
  parse-http-scheme-port-overflow
  parse-http-scheme-protocol-relative
  parse-http-scheme-query
  parse-http-scheme-query-percent-encode
  parse-http-scheme-relative-absolute-path
  parse-http-scheme-relative-bare-path
  parse-http-scheme-relative-path
  parse-http-scheme-relative-path-with-scheme
  parse-http-scheme-relative-percent-encoded-forbidden-host
  parse-http-scheme-relative-userinfo-idempotence
  parse-http-scheme-username
  parse-http-scheme-username-at-sign
  parse-http-scheme-username-password
  parse-http-scheme-username-password-percent-encode
  parse-http-scheme-username-percent-encode
  parse-invalid-host
  parse-invalid-port
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

add_executable(idna-conformance idna-conformance.c)

target_link_libraries(
  idna-conformance
  PRIVATE
    url_static
)

add_test(
  NAME idna-conformance
  COMMAND idna-conformance "${url_unicode_data}/IdnaTestV2.txt"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

set_tests_properties(
  idna-conformance
  PROPERTIES
  TIMEOUT 60
)

add_subdirectory(fuzz)
