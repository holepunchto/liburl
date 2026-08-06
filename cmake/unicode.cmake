# Generates the character tables that the domain parser needs to implement
# Unicode IDNA Compatibility Processing, as specified by UTS #46
# <https://www.unicode.org/reports/tr46>, from the Unicode Character Database
# that cmake-ucd fetches.
#
# This defines the `url_unicode_tables` target, which generates:
#
#   ${url_unicode_tables_header}  The declarations of the character tables.
#   ${url_unicode_tables_source}  The definitions of the character tables.

set(url_unicode_tables_header "${CMAKE_CURRENT_BINARY_DIR}/include/url/unicode-tables.h")
set(url_unicode_tables_source "${CMAKE_CURRENT_BINARY_DIR}/url/unicode-tables.c")

ucd_fetch(
  UCD
    UnicodeData.txt
    CompositionExclusions.txt
  PATHS url_unicode_sources
)

ucd_fetch(
  EXTRACTED
    DerivedBidiClass.txt
    DerivedGeneralCategory.txt
    DerivedJoiningType.txt
  PATHS url_unicode_sources
)

ucd_fetch(
  IDNA
    IdnaMappingTable.txt
  PATHS url_unicode_sources
)

if(PROJECT_IS_TOP_LEVEL)
  # The conformance test reads this, and nothing that is built does, so it is left
  # out of the tables the generator depends on.
  ucd_fetch(IDNA IdnaTestV2.txt)
endif()

find_program(NODE_EXECUTABLE NAMES node REQUIRED)

add_custom_command(
  OUTPUT
    "${url_unicode_tables_header}"
    "${url_unicode_tables_source}"
  COMMAND
    "${NODE_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate-unicode-tables.mjs"
    "${ucd_version}"
    "${ucd_data}"
    "${url_unicode_tables_header}"
    "${url_unicode_tables_source}"
  DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate-unicode-tables.mjs"
    ${url_unicode_sources}
  COMMENT "Generating Unicode character tables"
  VERBATIM
)

add_custom_target(url_unicode_tables DEPENDS "${url_unicode_tables_header}" "${url_unicode_tables_source}")
