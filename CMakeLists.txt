cmake_minimum_required(VERSION 4.0)

find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)
find_package(cmake-fuzz REQUIRED PATHS node_modules/cmake-fuzz)
find_package(cmake-prelude REQUIRED PATHS node_modules/cmake-prelude)
find_package(cmake-test REQUIRED PATHS node_modules/cmake-test)

project(url LANGUAGES C VERSION 0.0.0)

fetch_package("github:holepunchto/libutf")
fetch_package("github:holepunchto/libidna")

add_library_targets(
  url
  HEADERS
    include/url.h
    include/url/character-set.h
    include/url/infra.h
    include/url/parse.h
    include/url/percent-encode.h
    include/url/serialize.h
    include/url/type.h
  FEATURES
    c_std_11
  SOURCES
    src/url.c
  LINK
    utf::<TYPE>
    idna::<TYPE>
)

add_test_subdirectory(test)

if(PROJECT_IS_TOP_LEVEL)
  set(BENCH OFF CACHE BOOL "Build the benchmarks")

  if(BENCH)
    add_subdirectory(bench)
  endif()
endif()
