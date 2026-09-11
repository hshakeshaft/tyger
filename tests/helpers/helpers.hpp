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

#define ASSERT_STATEMENT_TYPE_IS(STMT, EXPECTED)                                \
    ASSERT_EQ((STMT)->type, EXPECTED)                                           \
    << "expected statement of type " << ast_statement_type_to_string(EXPECTED)  \
    << ", got " << ast_statement_type_to_string((STMT)->type)

#define ASSERT_EXPRESSION_TYPE_IS(EXPR, EXPECTED)                                    \
    ASSERT_EQ((EXPR)->type, EXPECTED)                                                \
        << "expected expression of type " << ast_expression_type_to_string(EXPECTED) \
        << ", got " << ast_expression_type_to_string((EXPR)->type)

Program test__parse_program_from_input(const char *input);
void parser__check_errors_and_log(Program program);
std::string program__expression_to_parse_tree(Program program, Expression *expr);
std::string program__statement_to_parse_tree(Program program, Statement *stmt);
std::string program__to_parse_tree(Program program);


////////////////////////////////////////////////////////////////////////////////
/// Eval Test Helpers

class EvalTestFixture : public ::testing::Test
{
protected:
    EvalTestFixture() {
        tyvm_init(&vm);
    }

    ~EvalTestFixture() {
        tyvm_deinit(&vm);
    }

    void init(const char *input) {
        lexer_init_from_buffer(&m_lexer, input);
        parser_init(&m_parser, &m_lexer);
        m_program = parser_parse_program(&m_parser);
        parser__check_errors_and_log(m_program);
    }

    // TODO(HS): destroy program here
    void reset() {}

    Lexer m_lexer;
    Parser m_parser;
    Program m_program;
    TyVM vm;
};


#define ASSERT_OBJECT_TYPE_IS(OBJ, EXPECTED)                                \
    ASSERT_EQ((OBJ)->type, EXPECTED)                                        \
        << "expected object of type " << tyobject_type_to_string(EXPECTED)  \
        << ", got " << tyobject_type_to_string((OBJ)->type)

#define ASSERT_OBJECT_EQ_AS_INT(OBJ, EXPECTED) \
    ASSERT_EQ((OBJ)->as.integer, EXPECTED)

#define ASSERT_OBJECT_EQ_AS_STRINGS(OBJ, EXPECTED)                              \
    do {                                                                        \
        ASSERT_EQ( (OBJ)->as.string.len, EXPECTED.length() );                   \
        auto actual_string = std::string(                                       \
            (OBJ)->as.string.str, (OBJ)->as.string.len                          \
        );                                                                      \
        ASSERT_EQ(actual_string, EXPECTED);                                     \
    } while (0)

#define ASSERT_OBJECT_IS_EQUAL_TO(OBJ, EXPECTED)                                \
    do {                                                                        \
        switch ((OBJ)->type) {                                                  \
            case OBJ_INTEGER: {                                                 \
                ASSERT_EQ( (OBJ)->as.integer, (EXPECTED).as_integer );          \
            } break;                                                            \
            case OBJ_STRING: {                                                  \
                ASSERT_EQ(                                                      \
                    std::string( (OBJ)->as.string.str, (OBJ)->as.string.len),   \
                    std::string( (EXPECTED).as_string )                         \
                );                                                              \
            } break;                                                            \
            default: {                                                          \
                FAIL();                                                         \
            }                                                                   \
        }                                                                       \
    } while (0)
