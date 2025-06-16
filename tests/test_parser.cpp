#include <gtest/gtest.h>
#include "tyger_test.hpp"
#include <string>

// TODO(HS): program deinit code
// TODO(HS): test error handling for invalid var statements
TEST(ParserTestSuite, Test_Var_Statement)
{
  const char *program = "var x = 10;";

  Lexer lx;
  lexer_init(&lx, program);

  Parser parser;
  parser_init(&parser, &lx);

  Program p = parser_parse_program(&parser);
  const char *prog_str = program_to_string(&p, TRACE_YAML);

  EXPECT_EQ(p.errors.len, 0) << "Expected 0 errors, got " << p.errors.len;
  if (p.errors.len > 0)
  {
    for (size_t i = 0; i < p.errors.len; ++i)
    {
      EXPECT_FALSE(true)
        << "[ERROR] " << tyger_error_kind_to_string(p.errors.elems[i].kind)
        << '\n';
    }
  }

  EXPECT_EQ(p.statements.len, 1) << prog_str;

  Statement *stmt = &(p.statements.elems[0]);
  EXPECT_EQ(stmt->kind, STMT_VAR)
    << "Expected Statement_Kind " << statement_kind_to_string(stmt->kind)
    << ", got " << statement_kind_to_string(STMT_VAR);

  std::string exp_ident{"x"};
  std::string act_ident{stmt->statement.var_statement.ident};
  EXPECT_EQ(exp_ident, act_ident) << prog_str;

  delete prog_str;
}
