cmake_minimum_required(VERSION 3.25)

set(PROJECT_NAME tyger_lang)
PROJECT(${PROJECT_NAME})

set(CMAKE_C_STANDARD 90)
set(CMAKE_CXX_STANDARD 17)
set(C_STANDARD_REQUIRED ON)
set(CXX_STANDARD_REQUIRED ON)


#
# NOTE(HS): Set some compiler flags for different OS/compilers
# TODO(HS): Add other compilers/options
# TODO(HS): Add switch for "release" vs "dev" builds
#
if (MSVC)
    add_compile_options(/W4 /w14640 /wd4996 /WX /permissive- /Zi /Od)
else()
    add_compile_options(-Wall -Wextra -Werror -Wshadow -Wno-unused-function -pedantic -g -O0)
endif()


#
# GTest setup
#   https://google.github.io/googletest/quickstart-cmake.html
#
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


#
# Build Tyger as static lib
#
set(LIB_NAME tyger)
set(
    LIB_SOURCES
    code/foo.c
)
add_library(${LIB_NAME} ${LIB_SOURCES})
target_include_directories(${LIB_NAME} PUBLIC includes)

#
# Build Tyger Main
#
set(MAIN_NAME tygeri)
set(
    MAIN_SOURCES
    code/main.c
)
add_executable(${MAIN_NAME} ${MAIN_SOURCES})
target_link_libraries(${MAIN_NAME} ${LIB_NAME})
target_include_directories(${MAIN_NAME} PUBLIC includes)

#
# Test Code
#
set(TEST_EXE test_tyger)
set(
    TEST_SOURCES
    tests/test_foo.cpp
)
add_executable(${TEST_EXE} ${TEST_SOURCES})
target_link_libraries(${TEST_EXE} GTest::gtest_main ${LIB_NAME})
target_include_directories(${TEST_EXE} PUBLIC includes)
include(GoogleTest)
gtest_discover_tests(${TEST_EXE})
