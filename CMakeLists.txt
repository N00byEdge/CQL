cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(CQL-test)

enable_language(C)
enable_language(CXX)

if(NOT (CMAKE_CXX_COMPILER_ID MATCHES MSVC)) #GCC
  set(CMAKE_CXX_FLAGS         "-std=c++17 -Werror -fsanitize=address,undefined -Wall -Wno-mismatched-tags -Wno-unknown-pragmas -Wno-sign-compare -Woverloaded-virtual -Wwrite-strings -Wno-unused")
  set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g3")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3")
  set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

include_directories(
  ${PROJECT_SOURCE_DIR}/CQL
)

set(GOOGLETEST_ROOT
  external/googletest/googletest CACHE STRING "Google Test source root"
)

include_directories(
  ${PROJECT_SOURCE_DIR}/${GOOGLETEST_ROOT}
  ${PROJECT_SOURCE_DIR}/${GOOGLETEST_ROOT}/include
)


set(GOOGLETEST_SOURCES
  ${PROJECT_SOURCE_DIR}/${GOOGLETEST_ROOT}/src/gtest-all.cc
  ${PROJECT_SOURCE_DIR}/${GOOGLETEST_ROOT}/src/gtest_main.cc
)

foreach(_source ${GOOGLETEST_SOURCES})
  set_source_files_properties(${_source} PROPERTIES GENERATED 1)
endforeach()

add_library(googletest
  ${GOOGLETEST_SOURCES}
)

file(GLOB testsources
	#${GOOGLETEST_SOURCES}
  ${PROJECT_SOURCE_DIR}/Tests/*.cpp
)

add_executable(tests
  ${testsources}
)

target_link_libraries(tests
  googletest
)

include(CTest)
enable_testing()

add_test(unit ${PROJECT_BINARY_DIR}/tests)
