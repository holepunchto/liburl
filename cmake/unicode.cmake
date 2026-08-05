# Fetches the Unicode Character Database and generates the character tables that
# the domain parser needs to implement Unicode IDNA Compatibility Processing, as
# specified by UTS #46 <https://www.unicode.org/reports/tr46>.
#
# This defines the `url_unicode_tables` target, which generates:
#
#   ${url_unicode_tables_header}  The declarations of the character tables.
#   ${url_unicode_tables_source}  The definitions of the character tables.

set(
  url_unicode_version 16.0.0
  CACHE STRING
  "The version of the Unicode Character Database to generate the character tables of the domain parser from"
)

set(url_unicode_data "${CMAKE_CURRENT_BINARY_DIR}/ucd/${url_unicode_version}")

set(url_unicode_tables_header "${CMAKE_CURRENT_BINARY_DIR}/include/url/unicode-tables.h")
set(url_unicode_tables_source "${CMAKE_CURRENT_BINARY_DIR}/url/unicode-tables.c")

function(url_fetch_unicode_data url name)
  if(EXISTS "${url_unicode_data}/${name}")
    return()
  endif()

  message(STATUS "Downloading ${name}")

  file(
    DOWNLOAD "${url}" "${url_unicode_data}/${name}"
    STATUS status
  )

  list(GET status 0 code)

  if(NOT code EQUAL 0)
    file(REMOVE "${url_unicode_data}/${name}")

    list(GET status 1 message)

    message(FATAL_ERROR "Could not download ${url}: ${message}")
  endif()
endfunction()

foreach(
  name IN ITEMS
    UnicodeData.txt
    CompositionExclusions.txt
)
  url_fetch_unicode_data(
    "https://www.unicode.org/Public/${url_unicode_version}/ucd/${name}"
    "${name}"
  )
endforeach()

foreach(
  name IN ITEMS
    DerivedBidiClass.txt
    DerivedGeneralCategory.txt
    DerivedJoiningType.txt
)
  url_fetch_unicode_data(
    "https://www.unicode.org/Public/${url_unicode_version}/ucd/extracted/${name}"
    "${name}"
  )
endforeach()

url_fetch_unicode_data(
  "https://www.unicode.org/Public/idna/${url_unicode_version}/IdnaMappingTable.txt"
  "IdnaMappingTable.txt"
)

if(PROJECT_IS_TOP_LEVEL)
  url_fetch_unicode_data(
    "https://www.unicode.org/Public/idna/${url_unicode_version}/IdnaTestV2.txt"
    "IdnaTestV2.txt"
  )
endif()

find_program(NODE_EXECUTABLE NAMES node REQUIRED)

add_custom_command(
  OUTPUT
    "${url_unicode_tables_header}"
    "${url_unicode_tables_source}"
  COMMAND
    "${NODE_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate-unicode-tables.mjs"
    "${url_unicode_version}"
    "${url_unicode_data}"
    "${url_unicode_tables_header}"
    "${url_unicode_tables_source}"
  DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate-unicode-tables.mjs"
    "${url_unicode_data}/CompositionExclusions.txt"
    "${url_unicode_data}/DerivedBidiClass.txt"
    "${url_unicode_data}/DerivedGeneralCategory.txt"
    "${url_unicode_data}/DerivedJoiningType.txt"
    "${url_unicode_data}/IdnaMappingTable.txt"
    "${url_unicode_data}/UnicodeData.txt"
  COMMENT "Generating Unicode character tables"
  VERBATIM
)

add_custom_target(url_unicode_tables DEPENDS "${url_unicode_tables_header}" "${url_unicode_tables_source}")
