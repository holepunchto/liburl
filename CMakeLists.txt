cmake_minimum_required(VERSION 3.25)

find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)

project(url C)

fetch_package("github:holepunchto/libutf")

add_library(url OBJECT)

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
    include/url/infra.h
    include/url/parse.h
    include/url/percent-encode.h
    include/url/serialize.h
    include/url/type.h
  PRIVATE
    src/url.c
)

target_include_directories(
  url
  PUBLIC
    include
    $<TARGET_PROPERTY:utf,INTERFACE_INCLUDE_DIRECTORIES>
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
)

install(TARGETS url_shared url_static)

install(FILES include/url.h DESTINATION include)

install(DIRECTORY include/url DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(bench)
  add_subdirectory(test)
endif()
