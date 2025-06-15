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
  const char *prog_str = program_to_string(&p);

  EXPECT_EQ(p.errors.len, 0) << "Expected 0 errors";
  EXPECT_EQ(p.statements.len, 1) << prog_str;

  // TODO(HS): test statement is var

  delete prog_str;
}
