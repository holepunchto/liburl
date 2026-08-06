cmake_minimum_required(VERSION 4.0)

find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)
find_package(cmake-fuzz REQUIRED PATHS node_modules/cmake-fuzz)
find_package(cmake-ucd REQUIRED PATHS node_modules/cmake-ucd)

project(url C)

fetch_package("github:holepunchto/libutf")
fetch_package("github:holepunchto/libpunycode")
fetch_package("github:holepunchto/libnormalize")

include(cmake/unicode.cmake)

add_library(url OBJECT)

add_dependencies(url url_unicode_tables)

set_target_properties(
  url
  PROPERTIES
  C_STANDARD 11
  POSITION_INDEPENDENT_CODE ON
)

target_sources(
  url
  INTERFACE
    include/url.h
    include/url/character-set.h
    include/url/idna.h
    include/url/infra.h
    include/url/parse.h
    include/url/percent-encode.h
    include/url/serialize.h
    include/url/type.h
    include/url/unicode.h
    "${url_unicode_tables_header}"
  PRIVATE
    src/url.c
    "${url_unicode_tables_source}"
)

target_include_directories(
  url
  PUBLIC
    include
    "${CMAKE_CURRENT_BINARY_DIR}/include"
    $<TARGET_PROPERTY:utf,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:punycode,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:normalize,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(url_shared SHARED)

set_target_properties(
  url_shared
  PROPERTIES
  OUTPUT_NAME url
  WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_link_libraries(
  url_shared
  PUBLIC
    url
    utf_shared
    punycode_shared
    normalize_shared
)

add_library(url_static STATIC)

set_target_properties(
  url_static
  PROPERTIES
  OUTPUT_NAME url
  PREFIX lib
)

target_link_libraries(
  url_static
  PUBLIC
    url
    utf_static
    punycode_static
    normalize_static
)

install(TARGETS url_shared url_static)

install(FILES include/url.h DESTINATION include)

install(DIRECTORY include/url DESTINATION include)

install(FILES "${url_unicode_tables_header}" DESTINATION include/url)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(bench)
  add_subdirectory(test)
endif()
