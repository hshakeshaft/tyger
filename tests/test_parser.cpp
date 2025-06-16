#include <gtest/gtest.h>
#include "tyger_test.hpp"

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
      EXPECT_FALSE(true) << "[ERROR] " << p.errors.elems[i].kind << '\n';
    }
  }

  EXPECT_EQ(p.statements.len, 1) << prog_str;

  // TODO(HS): test statement is var
  Statement *stmt = &(p.statements.elems[0]);
  EXPECT_EQ(stmt->kind, STMT_VAR)
    << "Expected Statement_Kind " << statement_kind_to_string(stmt->kind)
    << ", got " << statement_kind_to_string(STMT_VAR);

  delete prog_str;
}
