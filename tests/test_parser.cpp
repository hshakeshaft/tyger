#include <gtest/gtest.h>
#include "tyger_test.hpp"
#include <string>
#include <vector>
#include <cstdint>

// TODO(HS): pull this into own "test util" file
/// ScopeExit, adapted from the following Withness blog post to allow for defered blocks
/// http://the-witness.net/news/2012/11/scopeexit-in-c11/
template <typename F>
struct ScopeExit
{
  ScopeExit(F f) : f(f) {}
  ~ScopeExit() { f(); }
  F f;
};

template <typename F>
ScopeExit<F> MakeScopeExit(F f)
{
  return ScopeExit<F>(f);
}

#define DO_STRING_JOIN(S1, S2) S1 ## S2
#define STRING_JOIN_2(S1, S2) DO_STRING_JOIN(S1, S2)
#define DEFER(...) \
  auto STRING_JOIN_2(defer_scope_exit_, __LINE__) = MakeScopeExit([=] () { __VA_ARGS__ })

static void setup_parser_test_case(Parser *p, Lexer *lx, const char *program)
{
  lexer_init(lx, program);
  parser_init(p, lx);
}

static void parser_test_case_enumerate_errors(const Program *p, int expected_errors)
{
  EXPECT_EQ(expected_errors, p->errors.len)
    << "Expected " << expected_errors << " errors, got " << p->errors.len;

  if (p->errors.len > 0)
  {
    for (size_t i = 0; i < p->errors.len; ++i)
    {
      Tyger_Error *err = &(p->errors.elems[i]);
      EXPECT_FALSE(true) << "[ERROR] " << tyger_error_kind_to_string(err->kind) << '\n';
    }
  }
}

/// checks that a Statement is of the expected kind and logs an error if not
#define EXPECT_STATEMENT_IS(STMT, KIND)                                 \
  EXPECT_EQ((STMT)->kind, (KIND))                                       \
  << "Expected Statement_Kind " << statement_kind_to_string((KIND))     \
  << ", got " << statement_kind_to_string((STMT)->kind)

/// checks that an Expression is of the expected kind and logs an error if not
#define EXPECT_EXPRESSION_IS(EXPR, KIND)                                \
  EXPECT_EQ((EXPR)->kind, (KIND))                                       \
  << "Expected Expression_Kind " << expression_kind_to_string((KIND))   \
  << ", got " << expression_kind_to_string((EXPR)->kind)

// TODO(HS): add some more rigourous parsing example test cases

// TODO(HS): program deinit code
// TODO(HS): test error handling for invalid var statements
TEST(ParserTestSuite, Test_Var_Statement)
{
  Lexer lexer;
  Parser parser;
  const char *program = "var x = 10;";
  setup_parser_test_case(&parser, &lexer, program);

  Program p = parser_parse_program(&parser);
  const char *prog_str = program_to_string(&p, TRACE_YAML);
  DEFER(delete prog_str;);

  parser_test_case_enumerate_errors(&p, 0);
  EXPECT_EQ(p.statements.len, 1) << prog_str;

  Statement *stmt = &(p.statements.elems[0]);
  EXPECT_STATEMENT_IS(stmt, STMT_VAR);

  std::string exp_ident{"x"};
  std::string act_ident{stmt->statement.var_statement.ident};
  EXPECT_EQ(exp_ident, act_ident) << prog_str;
}

TEST(ParserTestSuite, Test_Int_Expression)
{
  Lexer lexer;
  Parser parser;
  const char *program = "10;";
  setup_parser_test_case(&parser, &lexer, program);

  Program p = parser_parse_program(&parser);
  const char *prog_str = program_to_string(&p, TRACE_YAML);
  DEFER(delete prog_str;);

  parser_test_case_enumerate_errors(&p, 0);
  EXPECT_EQ(p.statements.len, 1) << prog_str;

  Statement *stmt = &(p.statements.elems[0]);
  EXPECT_STATEMENT_IS(stmt, STMT_EXPRESSION);

  Expression *expr = stmt->statement.expression_statement.expression;
  EXPECT_EXPRESSION_IS(expr, EXPR_INT);

  int64_t exp_val = 10;
  int64_t act_val = expr->expression.int_expression.value;
  EXPECT_EQ(exp_val, act_val) << prog_str;
}

TEST(ParserTestSuite, Test_String_Expression)
{
  Lexer lexer;
  Parser parser;
  const char *program = "\"Hellope\";";
  setup_parser_test_case(&parser, &lexer, program);

  Program p = parser_parse_program(&parser);
  const char *prog_str = program_to_string(&p, TRACE_YAML);
  DEFER(delete prog_str;);

  parser_test_case_enumerate_errors(&p, 0);
  EXPECT_EQ(p.statements.len, 1) << prog_str;

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
    Lexer lexer;
    Parser parser;
    setup_parser_test_case(&parser, &lexer, tc.input);

    Program p = parser_parse_program(&parser);
    const char *prog_str = program_to_string(&p, TRACE_YAML);
    DEFER({delete prog_str;});

    parser_test_case_enumerate_errors(&p, 0);
    ASSERT_EQ(p.statements.len, 1) << prog_str;

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
    Lexer lexer;
    Parser parser;
    setup_parser_test_case(&parser, &lexer, tc.input);

    Program p = parser_parse_program(&parser);
    const char *prog_str = program_to_string(&p, TRACE_YAML);
    const char *act_ast = program_to_string(&p, TRACE_SEXPR);
    DEFER({
        delete prog_str;
        delete act_ast;
    });

    parser_test_case_enumerate_errors(&p, tc.num_errors);
    EXPECT_EQ(p.statements.len, tc.num_statements) << prog_str;

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
                                                     //       (-)
                                                     //      /   \
                                                     //    (+)    \
                                                     //   /   \    \
                                                     // (1)   (2)  (/)
                                                     //           /   \
                                                     //         (*)   (5)
                                                     //        /   \
                                                     //      (3)  (4)

    { "5 > 4 == 3 < 4;", "(== (> 5 4) (< 3 4))" },
    { "5 > 4 != 3 < 4;", "(!= (> 5 4) (< 3 4))" },

    // { "a + b * c + d / e - f;", "(- (+ (+ a (* b c)) (/ d e)) f)" },
                                                            //            (-)
                                                            //           /   \
                                                            //         (+)   (f)
                                                            //        /   \
                                                            //       /    (/)   
                                                            //      /    /   \
                                                            //    (+)  (d)   (e)
                                                            //   /   \
                                                            // (a)   (*)
                                                            //      /   \
                                                            //    (b)   (c)
  };

  for (auto& tc : test_cases)
  {
    Lexer lexer;
    Parser parser;
    setup_parser_test_case(&parser, &lexer, tc.input);

    Program p = parser_parse_program(&parser);
    const char *prog_str = program_to_string(&p, TRACE_YAML);
    const char *act_ast = program_to_string(&p, TRACE_SEXPR);
    DEFER({
        delete prog_str;
        delete act_ast;
    });

    parser_test_case_enumerate_errors(&p, 0);
    EXPECT_EQ(p.statements.len, 1) << prog_str;

    Statement *stmt = &(p.statements.elems[0]);
    EXPECT_STATEMENT_IS(stmt, STMT_EXPRESSION);

    Expression *expr = stmt->statement.expression_statement.expression;
    EXPECT_EXPRESSION_IS(expr, EXPR_INFIX);

    std::string act_ast_string{act_ast};
    std::string exp_ast_string{tc.ast};
    EXPECT_EQ(exp_ast_string, act_ast_string) << prog_str;
  }
}
