#pragma once
#include <gtest/gtest.h>

#include <iostream>
#include <ostream>
#include <vector>
#include <string>

#include "tyger.hpp"


////////////////////////////////////////////////////////////////////////////////
/// Lexer Test Helpers

std::ostream& operator<<(std::ostream& strm, const Lexer& l);
std::ostream& operator<<(std::ostream& strm, const Token& t);


////////////////////////////////////////////////////////////////////////////////
/// Parser Test Helpers

void parser__check_errors_and_log(Program program);
std::string program__expression_to_parse_tree(Program program, Expression *expr);
std::string program__to_parse_tree(Program program);


////////////////////////////////////////////////////////////////////////////////
/// Eval Test Helpers

#define ASSERT_OBJECT_TYPE_IS(OBJ, EXPECTED)                            \
    ASSERT_EQ((OBJ)->type, EXPECTED)                                    \
    << "expected object of type " << tyobject_type_to_string(EXPECTED)  \
    << ", got " << tyobject_type_to_string((OBJ)->type)

#define ASSERT_OBJECT_EQ_AS_INT(OBJ, EXPECTED) ASSERT_EQ((OBJ)->as.integer, EXPECTED)

#define ASSERT_OBJECT_EQ_AS_STRINGS(OBJ, EXPECTED)                                      \
    do {                                                                                \
        ASSERT_EQ( (OBJ)->as.string.len, EXPECTED.length() );                           \
        auto actual_string = std::string((OBJ)->as.string.str, (OBJ)->as.string.len);   \
        ASSERT_EQ(actual_string, EXPECTED);                                             \
    } while (0)

Program test__parse_program_from_input(const char *input);
