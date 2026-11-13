cmake_minimum_required(VERSION 4.0)

find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)
find_package(cmake-fuzz REQUIRED PATHS node_modules/cmake-fuzz)

project(url C)

fetch_package("github:holepunchto/libutf")
fetch_package("github:holepunchto/libidna")

get_property(shared GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)

add_library(url OBJECT)

set_target_properties(
  url
  PROPERTIES
  C_STANDARD 11
  POSITION_INDEPENDENT_CODE ${shared}
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
    $<TARGET_PROPERTY:idna,INTERFACE_INCLUDE_DIRECTORIES>
)

if(shared)
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
      idna_shared
  )
endif()

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
    idna_static
)

install(TARGETS url_static)

if(shared)
  install(TARGETS url_shared)
endif()

install(FILES include/url.h DESTINATION include)

install(DIRECTORY include/url DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  set(BENCH OFF CACHE BOOL "Build the benchmarks")

  enable_testing()

  if(BENCH)
    add_subdirectory(bench)
  endif()

  add_subdirectory(test)
endif()
