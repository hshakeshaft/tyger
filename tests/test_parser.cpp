#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstdint>
#include "../tests/parser_test_helper.hpp"

// TODO(HS): add some more rigourous parsing example test cases
// TODO(HS): program deinit code

// TODO(HS): test error handling for invalid var statements
TEST(ParserTestSuite, Test_Var_Statement)
{
  struct Var_Test
  {
    const char *input;
    const char *ident;
    int64_t value;
  };

  std::vector<Var_Test> test_cases{
    { "var x = 10;", "x", 10 },
    { "var y = 1;", "y", 1 },
    { "var fooBar = 15000;", "fooBar", 15000 },
    { "var spam_eggs = 8140124;", "spam_eggs", 8140124 },
  };

  for (auto& tc : test_cases)
  {
    SETUP_PARSER_TEST_CASE(tc.input);

    const char *prog_str = program_to_string((Program*) &p, TRACE_YAML);
    DEFER({
        delete prog_str;
        program_free((Program*) &p);
    });

    EXPECT_PROGRAM_PARSED_SUCCESS(p);
    ENUMERATE_PARSER_ERRORS(p);
    ASSERT_EQ(p.statements.len, 1) << prog_str;

    Statement *stmt = &(p.statements.elems[0]);
    EXPECT_STATEMENT_IS(stmt, STMT_VAR);

    std::string exp_ident{tc.ident};
    std::string act_ident{stmt->statement.var_statement.ident};
    EXPECT_EQ(exp_ident, act_ident) << prog_str;

    Expression *expr = stmt->statement.var_statement.expression;
    ASSERT_NE(expr, nullptr) << prog_str;
    ASSERT_EQ(expr->kind, EXPR_INT) << prog_str;
    EXPECT_EQ(expr->expression.int_expression.value, tc.value) << prog_str;
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
  const char *program = "\"Hellope\";";
  SETUP_PARSER_TEST_CASE(program);

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
  EXPECT_EXPRESSION_IS(expr, EXPR_STRING);

  std::string exp_value{"Hellope"};
  std::string act_value{expr->expression.string_expression.value};
  EXPECT_EQ(exp_value, act_value) << prog_str;
  EXPECT_EQ(exp_value.size(), act_value.size()) << prog_str;
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
