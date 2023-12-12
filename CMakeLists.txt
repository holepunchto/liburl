cmake_minimum_required(VERSION 3.25)

project(url C)

if(NOT TARGET utf)
  add_subdirectory(vendor/libutf EXCLUDE_FROM_ALL)
endif()

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
  PRIVATE
    src/infra.h
    src/parse.h
    src/percent-encode.h
    src/percent-encode-set.h
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

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(bench)
  add_subdirectory(test)
endif()
