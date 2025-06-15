cmake_minimum_required(VERSION 3.16)
set(PROJECT_NAME tyger)

PROJECT(${PROJECT_NAME})

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)
set(C_STANDARD_REQUIRED ON)


#
# NOTE(HS): Set some compiler flags for different OS/compilers
# TODO(HS): Add other compilers/options
# TODO(HS): Add switch for "release" vs "dev" builds
#
if (MSVC)
    add_compile_options(/W4 /w14640 /WX /permissive- /Zi /Od)
else()
    add_compile_options(-Wall -Wextra -Werror -Wshadow -pedantic -g -O0)
endif()


#
# GTest setup - https://google.github.io/googletest/quickstart-cmake.html
#
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# NOTE(HS): for windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


#
# Build "lib"
#
set(LIB_NAME ${PROJECT_NAME}_lib)
set(
    LIB_SOURCES
    code/lexer.c
    code/tstrings.c
    code/repl.c
    code/parser.c
)
add_library(${LIB_NAME} STATIC ${LIB_SOURCES})
target_include_directories(${LIB_NAME} PUBLIC includes)


#
# Build exe
#
set(EXE_SOURCES code/main.c)
add_executable(${PROJECT_NAME} ${EXE_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC includes)
target_link_libraries(${PROJECT_NAME} ${LIB_NAME})

#
# GTest
#
enable_testing()
set(TEST_EXE ${PROJECT_NAME}_test)
set(
    TEST_SOURCES
    tests/test_lexer.cpp
    tests/test_parser.cpp
)

add_executable(${TEST_EXE} ${TEST_SOURCES})
target_include_directories(${TEST_EXE} PUBLIC includes)

target_link_libraries(
    ${TEST_EXE}
    GTest::gtest_main
    ${LIB_NAME}
)

include(GoogleTest)
gtest_discover_tests(${TEST_EXE})
