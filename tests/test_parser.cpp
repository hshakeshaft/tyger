#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstdint>
#include "../tests/parser_test_helper.hpp"

// TODO(HS): add some more rigourous parsing example test cases
// TODO(HS): test error handling for invalid var statements

// TODO(HS): might be able to just run asserts on the `sexpr` trace form,
TEST(ParserTestSuite, Test_Var_Statement)
{
  struct Var_Test
  {
    const char *input;
    std::string ident;
    std::string ast;
  };

  std::vector<Var_Test> test_cases{
    { "var x = 10;", "x", "(var x 10)" },
    { "var y = 15000;", "y", "(var y 15000)" },
    { "var msg = \"Hello, Sunshine! The Earth says Hello!\";", "msg", "(var msg \"Hello, Sunshine! The Earth says Hello!\")" },
    { "var theQuickBrownFoxJumpsOverTheLazyDog123456789 = 1;", "theQuickBrownFoxJumpsOverTheLazyDog123456789", "(var theQuickBrownFoxJumpsOverTheLazyDog123456789 1)", },
  };

  for (auto& tc : test_cases)
  {
    SETUP_PARSER_TEST_CASE(tc.input);

    const char *prog_str = program_to_string((Program*) &p, TRACE_YAML);
    const char *prog_sexpr = program_to_string((Program*) &p, TRACE_SEXPR);
    DEFER({
        delete prog_str;
        delete prog_sexpr;
        program_free((Program*) &p);
    });

    EXPECT_PROGRAM_PARSED_SUCCESS(p);
    ENUMERATE_PARSER_ERRORS(p);
    ASSERT_EQ(p.statements.len, 1) << prog_str;

    Statement *stmt = &(p.statements.elems[0]);
    EXPECT_STATEMENT_IS(stmt, STMT_VAR);

    const Var_Statement *vs = &stmt->statement.var_statement;

    const char *ident = ident_handle_to_ident(&p, vs->ident_handle);
    ASSERT_NE(ident, nullptr);

    std::string act_ident{ident};
    ASSERT_EQ(tc.ident, act_ident);

    // NOTE(HS): Allows for more convienient testing of expressions assigned as part
    // of var_statements
    std::string act_ast{prog_sexpr};
    ASSERT_EQ(tc.ast, act_ast);
  }
}

TEST(ParserTestSuite, Test_Int_Expression)
{
  const char *program = "10;";
  SETUP_PARSER_TEST_CASE(program);

  const char *prog_str = program_to_string((Program*) &p, TRACE_YAML);
  DEFER({
      delete prog_str;
      program_free((Program*) &p);
  });

  EXPECT_PROGRAM_PARSED_SUCCESS(p);
  ENUMERATE_PARSER_ERRORS(p);

  Statement *stmt = &(p.statements.elems[0]);
  EXPECT_STATEMENT_IS(stmt, STMT_EXPRESSION) << prog_str;

  Expression *expr = stmt->statement.expression_statement.expression;
  EXPECT_EXPRESSION_IS(expr, EXPR_INT) << prog_str;

  int64_t exp_val = 10;
  int64_t act_val = expr->expression.int_expression.value;
  EXPECT_EQ(exp_val, act_val) << prog_str;
}

TEST(ParserTestSuite, Test_String_Expression)
{
  struct String_Test
  {
    std::string input;
    std::string expected;
  };

  std::vector<String_Test> test_cases{
    { "\"\";", "" },
    { "\"foo\";", "foo" },
    { "\"bar\";", "bar" },
    { "\"baz\";", "baz" },
    { "\"Hello, World!\";", "Hello, World!" },

    // NOTE(HS): this is to test longer strings and reallocations under the hood
    { "\"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\";", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" },
    { "\"the quick brown fox jumps over the lazy dog\";", "the quick brown fox jumps over the lazy dog" },
  };

  for (auto& tc : test_cases)
  {
    SETUP_PARSER_TEST_CASE(tc.input.c_str());
    const char *prog_str = program_to_string(&p, TRACE_YAML);
    DEFER({
        delete prog_str;
        program_free((Program*) &p);
    });

    EXPECT_PROGRAM_PARSED_SUCCESS(p);
    ENUMERATE_PARSER_ERRORS(p);

    Statement *stmt = &(p.statements.elems[0]);
    EXPECT_STATEMENT_IS(stmt, STMT_EXPRESSION);

    Expression *expr = stmt->statement.expression_statement.expression;
    EXPECT_EXPRESSION_IS(expr, EXPR_STRING);

    String_Expression se = expr->expression.string_expression;
    std::string act_value{se.value};
    EXPECT_EQ(tc.expected, act_value) << prog_str;
    EXPECT_EQ(tc.expected.size(), act_value.size()) << prog_str;
  }
}

TEST(ParserTestSuite, Test_Ident_Expression)
{
  struct Ident_Test
  {
    const char *input;
    const char *ident;
  };

  std::vector<Ident_Test> test_cases{
    { "x;", "x" },
    { "foo;", "foo" },
    { "fooBarBaz;", "fooBarBaz" },
  };

  for (auto& tc : test_cases)
  {
    SETUP_PARSER_TEST_CASE(tc.input);

    const char *prog_str = program_to_string((Program*) &p, TRACE_YAML);
    DEFER({
        delete prog_str;
        program_free((Program*) &p);
    });

    EXPECT_PROGRAM_PARSED_SUCCESS(p) << prog_str;
    ENUMERATE_PARSER_ERRORS(p);

    Statement *stmt = &(p.statements.elems[0]);
    EXPECT_STATEMENT_IS(stmt, STMT_EXPRESSION);

    Expression *expr = stmt->statement.expression_statement.expression;
    EXPECT_EXPRESSION_IS(expr, EXPR_IDENT);

    std::string exp_ident{tc.ident};
    std::string act_ident{expr->expression.ident_expression.ident};
    EXPECT_EQ(exp_ident, act_ident);
  }
}

TEST(ParserTestSuite, Test_Infix_Expression)
{
  struct Infix_Test
  {
    const char *input;
    int num_errors;
    int num_statements;
    const char *ast;    // S-expression like formatted representation of AST
  };

  std::vector<Infix_Test> test_cases{
    { "1 + 1;",  0, 1, "(+ 1 1)" },
    { "1 - 1;",  0, 1, "(- 1 1)" },
    { "1 * 1;",  0, 1, "(* 1 1)" },
    { "1 / 1;",  0, 1, "(/ 1 1)" },
    { "1 < 1;",  0, 1, "(< 1 1)" },
    { "1 > 1;",  0, 1, "(> 1 1)" },
    { "1 == 1;", 0, 1, "(== 1 1)" },
    { "1 != 1;", 0, 1, "(!= 1 1)" },
  };

  for (auto& tc : test_cases)
  {
    SETUP_PARSER_TEST_CASE(tc.input);

    const char *prog_str = program_to_string((Program*) &p, TRACE_YAML);
    const char *act_ast = program_to_string((Program*) &p, TRACE_SEXPR);
    DEFER({
        delete prog_str;
        delete act_ast;
        program_free((Program*) &p);
    });

    EXPECT_PROGRAM_PARSED_SUCCESS(p);
    ENUMERATE_PARSER_ERRORS(p);

    Statement *stmt = &(p.statements.elems[0]);
    EXPECT_STATEMENT_IS(stmt, STMT_EXPRESSION);

    Expression *expr = stmt->statement.expression_statement.expression;
    EXPECT_EXPRESSION_IS(expr, EXPR_INFIX);

    std::string act_ast_string{act_ast};
    std::string exp_ast_string{tc.ast};
    EXPECT_EQ(exp_ast_string, act_ast_string) << prog_str;
  }
}

// TODO(HS): some more rigourous test cases to robustly check arbitrary nesting
// TODO(HS): add logical operators to test cases
TEST(ParserTestSuite, Test_Operator_Precidence)
{
  struct Precidence_Test
  {
    const char *input;
    const char *ast;
  };

  std::vector<Precidence_Test> test_cases{
    { "1 + 2 + 3;", "(+ (+ 1 2) 3)" },
    { "1 - 2 - 3;", "(- (- 1 2) 3)" },
    { "1 + 2 - 3;", "(- (+ 1 2) 3)" },

    { "1 * 2;", "(* 1 2)" },
    { "4 / 2;", "(/ 4 2)" },
    { "4 * 3 / 2", "(/ (* 4 3) 2)" },

    { "1 + 2 * 3;", "(+ 1 (* 2 3))" },
    { "4 + 9 / 3;", "(+ 4 (/ 9 3))" },

    { "1 + 2 - 3 * 4 / 5;", "(- (+ 1 2) (/ (* 3 4) 5))" },
                                                     /*
                                                            (-)
                                                           /   \
                                                         (+)    \
                                                        /   \    \
                                                      (1)   (2)  (/)
                                                                /   \
                                                              (*)   (5)
                                                             /   \
                                                           (3)  (4)
                                                     */

    { "5 > 4 == 3 < 4;", "(== (> 5 4) (< 3 4))" },
    { "5 > 4 != 3 < 4;", "(!= (> 5 4) (< 3 4))" },

    { "a + b * c + d / e - f;", "(- (+ (+ a (* b c)) (/ d e)) f)" },
                                                            /*
                                                                        (-)
                                                                       /   \
                                                                     (+)   (f)
                                                                    /   \
                                                                   /    (/)
                                                                  /    /   \
                                                                (+)  (d)   (e)
                                                               /   \
                                                             (a)   (*)
                                                                  /   \
                                                                (b)   (c)
                                                            */
  };

  for (auto& tc : test_cases)
  {
    SETUP_PARSER_TEST_CASE(tc.input);

    const char *prog_str = program_to_string((Program*) &p, TRACE_YAML);
    const char *act_ast = program_to_string((Program*) &p, TRACE_SEXPR);
    DEFER({
        delete prog_str;
        delete act_ast;
        program_free((Program*) &p);
    });

    EXPECT_PROGRAM_PARSED_SUCCESS(p);
    ENUMERATE_PARSER_ERRORS(p);

    Statement *stmt = &(p.statements.elems[0]);
    EXPECT_STATEMENT_IS(stmt, STMT_EXPRESSION) << prog_str;

    Expression *expr = stmt->statement.expression_statement.expression;
    EXPECT_EXPRESSION_IS(expr, EXPR_INFIX) << prog_str;

    std::string act_ast_string{act_ast};
    std::string exp_ast_string{tc.ast};
    EXPECT_EQ(exp_ast_string, act_ast_string) << prog_str;
  }
}

// TODO(HS): add more test cases when support added for other function idents
TEST(ParserTestSuite, Test_Call_Expression)
{
  struct Function_Test
  {
    const char *input;
    const char *ident;
    std::size_t arg_count;
    const char *ast;
  };

  std::vector<Function_Test> test_cases{
    { "println();", "println", 0, "(println [])" },
    { "println(5);", "println", 1, "(println [5])" },
    { "println(\"Hello, World!\");", "println", 1, "(println [\"Hello, World!\"])" },
    { "println(\"a\");", "println", 1, "(println [\"a\"])" },
    { "println(1 + 2);", "println", 1, "(println [(+ 1 2)])" },
    { "println(1 + 2 - 3 * 4 / 5);", "println", 1, "(println [(- (+ 1 2) (/ (* 3 4) 5))])" },
    { "println(1, 1);", "println", 2, "(println [1 ; 1])"  },
    { "println(5, 4, 3, 2, 1);", "println", 5, "(println [5 ; 4 ; 3 ; 2 ; 1])"  },
    { "println(5, 1 + 1, \"fooBar\");", "println", 3, "(println [5 ; (+ 1 1) ; \"fooBar\"])"  },
  };

  for (auto& tc : test_cases)
  {
    SETUP_PARSER_TEST_CASE(tc.input);
    const char *prog_str = program_to_string((Program*) &p, TRACE_YAML);
    const char *act_ast = program_to_string((Program*) &p, TRACE_SEXPR);
    DEFER({
        delete prog_str;
        delete act_ast;
        program_free((Program*) &p);
    });

    EXPECT_PROGRAM_PARSED_SUCCESS(p);
    ENUMERATE_PARSER_ERRORS(p);

    Statement *stmt = &(p.statements.elems[0]);
    EXPECT_STATEMENT_IS(stmt, STMT_EXPRESSION) << prog_str;

    Expression *expr = stmt->statement.expression_statement.expression;
    EXPECT_EXPRESSION_IS(expr, EXPR_CALL) << prog_str;

    Call_Expression *ce = &(expr->expression.call_expression);
    ASSERT_EQ(ce->args.len, tc.arg_count) << prog_str;

    std::string act_ast_string{act_ast};
    std::string exp_ast_string{tc.ast};
    EXPECT_EQ(act_ast_string, exp_ast_string) << prog_str;
  }
}
