#include <gtest/gtest.h>

#include <iostream>
#include <vector>
#include <vector>

#include "parser.h"

struct IntegerExpressionTestCase
{
    std::string input;
    int expected;
};


TEST(ParserTestSuite, Integer_Expression)
{
    std::vector<IntegerExpressionTestCase> test_cases = {
        { "10;", 10 },
    };

    for (auto& tc : test_cases)
    {
        Lexer lexer;
        Parser parser;

        lexer_init_from_buffer(&lexer, tc.input.c_str());
        parser_init(&parser, &lexer);

        Program program = parser_parse_program(&parser);

        if (program.errors.count != 0)
        {
            for (size_t i = 0; i < program.errors.count; ++i)
            {
                Error *cur_err;
                cur_err = &program.errors.elems[i];
                std::cerr << "ERROR :: " << cur_err->what << '\n';
            }
            ASSERT_EQ(program.errors.count, 0);
        }

        Statement_Handle stmt_handle = {1};
        Statement *stmt = program_statement_handle_to_statement(&program, stmt_handle);
        ASSERT_EQ(stmt->type, ST_EXPRESSION)
            << "Expected statement of type " << ast_statement_type_to_string(ST_EXPRESSION)
            << ", got " << ast_statement_type_to_string(stmt->type);

        Expression_Handle expr_handle = stmt->as.expression.handle;
        Expression *expr = program_expression_handle_to_expression(&program, expr_handle);
        ASSERT_EQ(expr->type, ET_INTEGER)
            << "Expected expression of type " << ast_expression_type_to_string(ET_INTEGER)
            << ", got " << ast_expression_type_to_string(expr->type);

        ASSERT_EQ(expr->as.integer.value, tc.expected);
    }
}
