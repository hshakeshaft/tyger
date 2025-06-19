#include <gtest/gtest.h>
#include "tyger_test.hpp"
#include <string>
#include <cstdint>

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

static void parser_test_case_enumerate_errors(const Program *p)
{
  if (p->errors.len > 0)
  {
    for (size_t i = 0; i < p->errors.len; ++i)
    {
      Tyger_Error *err = &(p->errors.elems[i]);
      EXPECT_FALSE(true) << "[ERROR] " << tyger_error_kind_to_string(err->kind) << '\n';
    }
  }
}

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

  EXPECT_EQ(p.errors.len, 0) << "Expected 0 errors, got " << p.errors.len;
  parser_test_case_enumerate_errors(&p);

  EXPECT_EQ(p.statements.len, 1) << prog_str;

  Statement *stmt = &(p.statements.elems[0]);
  EXPECT_EQ(stmt->kind, STMT_VAR)
    << "Expected Statement_Kind " << statement_kind_to_string(stmt->kind)
    << ", got " << statement_kind_to_string(STMT_VAR);

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

  EXPECT_EQ(p.errors.len, 0) << "Expected 0 errors, got " << p.errors.len;
  parser_test_case_enumerate_errors(&p);

  EXPECT_EQ(p.statements.len, 1) << prog_str;

  Statement *stmt = &(p.statements.elems[0]);
  EXPECT_EQ(stmt->kind, STMT_EXPRESSION)
    << "Expected Statement_Kind " << statement_kind_to_string(STMT_EXPRESSION)
    << ", got " << statement_kind_to_string(stmt->kind)
    << prog_str;

  Expression *expr = stmt->statement.expression_statement.expression;
  EXPECT_EQ(expr->kind, EXPR_INT)
    << "Expected Expression_Kind " << expression_kind_to_string(EXPR_INT)
    << ", got " << expression_kind_to_string(expr->kind)
    << prog_str;

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

  EXPECT_EQ(p.errors.len, 0) << "Expected 0 errors, got " << p.errors.len;
  parser_test_case_enumerate_errors(&p);

  EXPECT_EQ(p.statements.len, 1) << prog_str;

  Statement *stmt = &(p.statements.elems[0]);
  EXPECT_EQ(stmt->kind, STMT_EXPRESSION)
    << "Expected Statement_Kind " << statement_kind_to_string(STMT_EXPRESSION)
    << ", got " << statement_kind_to_string(stmt->kind)
    << prog_str;

  Expression *expr = stmt->statement.expression_statement.expression;
   EXPECT_EQ(expr->kind, EXPR_STRING)
    << "Expected Expression_Kind " << expression_kind_to_string(EXPR_STRING)
    << ", got " << expression_kind_to_string(expr->kind)
    << prog_str;

   std::string exp_value{"Hellope"};
   std::string act_value{expr->expression.string_expression.value};
   EXPECT_EQ(exp_value, act_value) << prog_str;
   EXPECT_EQ(exp_value.size(), act_value.size()) << prog_str;
}
