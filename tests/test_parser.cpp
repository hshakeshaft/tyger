#include <gtest/gtest.h>

#include <iostream>
#include <vector>
#include <vector>

#include "tyger.hpp"

struct IntegerExpressionTestCase
{
    std::string input;
    int expected;
};

struct StringExpressionTestCase
{
    std::string input;
    std::string expected;
};

struct IdentExpressionTestCase
{
    std::string input;
    std::string expected;
};

struct InfixExpressionTestCase
{
    std::string input;
    std::string expected_parse_tree;
};

static void parser__check_errors_and_log(Program program)
{
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
}

static std::string program__expression_to_parse_tree(Program program, Expression *expr)
{
    std::string result{};

    switch (expr->type)
    {
        case ET_INTEGER: {
            result += std::to_string(expr->as.integer.value);
        } break;

        case ET_STRING: {
            result += std::string(expr->as.string.ptr);
        } break;

        case ET_IDENT: {
            result += std::string(expr->as.ident.name);
        } break;

        case ET_INFIX: {
            result += "(";

            Infix_Expression *infix = &expr->as.infix;
            switch (infix->op)
            {
                case OP_ADD: result += "+ "; break;
                case OP_SUB: result += "- "; break;
                case OP_MUL: result += "* "; break;
                case OP_DIV: result += "/ "; break;
                default:;
            }

            Expression *lhs = program_expression_handle_to_expression(&program, infix->lhs);
            Expression *rhs = program_expression_handle_to_expression(&program, infix->rhs);

            result += program__expression_to_parse_tree(program, lhs);
            result += " ";
            result += program__expression_to_parse_tree(program, rhs);

            result += ")";
        } break;

        default:;
    }

    return result;
}

static std::string program__to_parse_tree(Program program)
{
    std::string result{};

    for (size_t i = 1; i < program.___statements.count; ++i)
    {
        auto stmt = &program.___statements.elems[i];
        switch (stmt->type)
        {
            case ST_EXPRESSION: {
                Expression *expr = program_expression_handle_to_expression(&program, stmt->as.expression.handle);
                result = program__expression_to_parse_tree(program, expr);
            } break;

            default:;
        }
    }

    return result;
}


TEST(ParserTestSuite, Integer_Expression)
{
    std::vector<IntegerExpressionTestCase> test_cases = {
        { "10;", 10 },
        { "100000;", 100000 },
        { "5;", 5 }
    };

    for (auto& tc : test_cases)
    {
        Lexer lexer;
        Parser parser;

        lexer_init_from_buffer(&lexer, tc.input.c_str());
        parser_init(&parser, &lexer);

        Program program = parser_parse_program(&parser);

        parser__check_errors_and_log(program);

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

TEST(ParserTestSuite, String_Expression)
{
    std::vector<StringExpressionTestCase> test_cases = {
        { "\"Henlo!\";", "Henlo!" },
        { "\"the quick brown fox jumps over the lazy dog\";", "the quick brown fox jumps over the lazy dog" },
    };

    for (auto& tc : test_cases)
    {
        Lexer lexer;
        Parser parser;
        lexer_init_from_buffer(&lexer, tc.input.c_str());
        parser_init(&parser, &lexer);

        Program program = parser_parse_program(&parser);

        parser__check_errors_and_log(program);

        Statement *stmt = program_statement_handle_to_statement(&program, {1});
        ASSERT_NE(stmt, nullptr);
        ASSERT_EQ(stmt->type, ST_EXPRESSION)
            << "Expected expression of type " << ast_statement_type_to_string(ST_EXPRESSION)
            << ", got " << ast_statement_type_to_string(stmt->type);

        Expression *expr = program_expression_handle_to_expression(&program, stmt->as.expression.handle);
        ASSERT_NE(expr, nullptr);
        ASSERT_EQ(expr->type, ET_STRING);
        ASSERT_EQ(std::string(expr->as.string.ptr, expr->as.string.len), tc.expected);
    }
}

TEST(ParserTestSuite, Ident_Expression)
{
    auto test_cases = std::vector<IdentExpressionTestCase>{
        { "x;", "x" },
        { "fooBar;", "fooBar" },
    };

    for (auto& tc : test_cases)
    {
        Lexer lexer;
        Parser parser;
        lexer_init_from_buffer(&lexer, tc.input.c_str());
        parser_init(&parser, &lexer);

        Program program = parser_parse_program(&parser);
        parser__check_errors_and_log(program);

        Statement *stmt = program_statement_handle_to_statement(&program, {1});
        ASSERT_NE(stmt, nullptr);
        ASSERT_EQ(stmt->type, ST_EXPRESSION)
            << "expected statement of type " << ast_statement_type_to_string(ST_EXPRESSION)
            << ", got " << ast_statement_type_to_string(stmt->type);

        Expression *expr = program_expression_handle_to_expression(&program, stmt->as.expression.handle);
        ASSERT_NE(expr, nullptr);
        ASSERT_EQ(expr->type, ET_IDENT)
            << "expected expression of type " << ast_expression_type_to_string(ET_IDENT)
            << ", got " << ast_expression_type_to_string(expr->type);

        ASSERT_EQ(expr->as.ident.name, tc.expected);
    }
}

TEST(ParserTestSuite, Infix_Expression)
{
    auto test_cases = std::vector<InfixExpressionTestCase>{
        { "1 + 1;", "(+ 1 1)" },
        { "1 - 1;", "(- 1 1)" },
        { "1 * 1;", "(* 1 1)" },
        { "1 / 1;", "(/ 1 1)" },
    };

    for (auto& tc : test_cases)
    {
        lexer lexer;
        parser parser;
        lexer_init_from_buffer(&lexer, tc.input.c_str());
        parser_init(&parser, &lexer);

        program program = parser_parse_program(&parser);
        parser__check_errors_and_log(program);

        std::string parse_tree = program__to_parse_tree(program);
        ASSERT_EQ(tc.expected_parse_tree, parse_tree);
    }
}

TEST(ParserTestSuite, Operator_Precidence)
{
    auto test_cases = std::vector<InfixExpressionTestCase>{
        { "1 + 2 + 3;", "(+ (+ 1 2) 3)" },
        { "1 - 2 - 3;", "(- (- 1 2) 3)" },
        { "3 * 4 / 2;", "(/ (* 3 4) 2)" },
        { "1 + 2 * 3;", "(+ 1 (* 2 3))" },
        { "1 + 2 / 3;", "(+ 1 (/ 2 3))" },
        { "1 + 2 - 3 * 4 / 5;", "(- (+ 1 2) (/ (* 3 4) 5))" },
        { "a + b - c * d / e;", "(- (+ a b) (/ (* c d) e))" },
        { "1 * 2 / 3 - 4 + 5;", "(+ (- (/ (* 1 2) 3) 4) 5)" },
    };

    for (auto& tc : test_cases)
    {
        lexer lexer;
        parser parser;
        lexer_init_from_buffer(&lexer, tc.input.c_str());
        parser_init(&parser, &lexer);

        program program = parser_parse_program(&parser);
        parser__check_errors_and_log(program);

        std::string parse_tree = program__to_parse_tree(program);
        ASSERT_EQ(tc.expected_parse_tree, parse_tree);
    }
}
