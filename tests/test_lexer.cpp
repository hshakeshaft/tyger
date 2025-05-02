#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <stddef.h>
#include "lexer.h"
#include "tstrings.h"

// TODO(HS): eliniate leading space before integers
const char *INPUT = \
  "(){}[];\n"
  "+ - * / !\n"
  "!= == < > <= >=\n"
  " \n"
  "10;\n"
  "100;\n"
  "0;\n"
  "\n"
  "\"Hello, World!\";\n"
  "\"Hello \\\"World\\\"\";\n"
  "\"\";\n"
  "\n"
  "var x = 10;\n"
  "println();\n"
;

struct Literal
{
  char *str;
  size_t len;
};

struct Lexer_Test_Case
{
  Location location;
  Literal literal;
  Token_Kind kind;
};

const auto TEST_CASES = std::vector<Lexer_Test_Case>{
  Lexer_Test_Case{ Location{ 0, 0, 0 }, Literal{ (char*) "(", 1 }, TK_LPAREN },
  Lexer_Test_Case{ Location{ 1, 0, 0 }, Literal{ (char*) ")", 1 }, TK_RPAREN },
  Lexer_Test_Case{ Location{ 2, 0, 0 }, Literal{ (char*) "{", 1 }, TK_LBRACE },
  Lexer_Test_Case{ Location{ 3, 0, 0 }, Literal{ (char*) "}", 1 }, TK_RBRACE },
  Lexer_Test_Case{ Location{ 4, 0, 0 }, Literal{ (char*) "[", 1 }, TK_LBRACKET },
  Lexer_Test_Case{ Location{ 5, 0, 0 }, Literal{ (char*) "]", 1 }, TK_RBRACKET },
  Lexer_Test_Case{ Location{ 6, 0, 0 }, Literal{ (char*) ";", 1 }, TK_SEMICOLON },
  Lexer_Test_Case{ Location{ 8, 0, 0 }, Literal{ (char*) "+", 1 }, TK_PLUS },
  Lexer_Test_Case{ Location{ 10, 0, 0 }, Literal{ (char*) "-", 1 }, TK_MINUS },
  Lexer_Test_Case{ Location{ 12, 0, 0 }, Literal{ (char*) "*", 1 }, TK_ASTERISK },
  Lexer_Test_Case{ Location{ 14, 0, 0 }, Literal{ (char*) "/", 1 }, TK_SLASH },
  Lexer_Test_Case{ Location{ 16, 0, 0 }, Literal{ (char*) "!", 1 }, TK_BANG },
  Lexer_Test_Case{ Location{ 18, 0, 0 }, Literal{ (char*) "!=", 2 }, TK_NOT_EQ },
  Lexer_Test_Case{ Location{ 21, 0, 0 }, Literal{ (char*) "==", 2 }, TK_EQ },
  Lexer_Test_Case{ Location{ 24, 0, 0 }, Literal{ (char*) "<", 1 }, TK_LT },
  Lexer_Test_Case{ Location{ 26, 0, 0 }, Literal{ (char*) ">", 1 }, TK_GT },
  Lexer_Test_Case{ Location{ 28, 0, 0 }, Literal{ (char*) "<=", 2 }, TK_LTE },
  Lexer_Test_Case{ Location{ 31, 0, 0 }, Literal{ (char*) ">=", 2 }, TK_GTE },
  Lexer_Test_Case{ Location{ 35, 0, 0 }, Literal{ (char*) "10", 2 }, TK_INTEGER },
  Lexer_Test_Case{ Location{ 37, 0, 0 }, Literal{ (char*) ";", 1 }, TK_SEMICOLON },
  Lexer_Test_Case{ Location{ 39, 0, 0 }, Literal{ (char*) "100", 3 }, TK_INTEGER },
  Lexer_Test_Case{ Location{ 42, 0, 0 }, Literal{ (char*) ";", 1 }, TK_SEMICOLON },
  Lexer_Test_Case{ Location{ 44, 0, 0 }, Literal{ (char*) "0", 1 }, TK_INTEGER },
  Lexer_Test_Case{ Location{ 45, 0, 0 }, Literal{ (char*) ";", 1 }, TK_SEMICOLON },
  Lexer_Test_Case{ Location{ 48, 0, 0 }, Literal{ (char*) "Hello, World!", 13 }, TK_STRING },
  Lexer_Test_Case{ Location{ 63, 0, 0 }, Literal{ (char*) ";", 1 }, TK_SEMICOLON },
  Lexer_Test_Case{ Location{ 65, 0, 0 }, Literal{ (char*) "Hello \\\"World\\\"", 15 }, TK_STRING },
  Lexer_Test_Case{ Location{ 82, 0, 0 }, Literal{ (char*) ";", 1 }, TK_SEMICOLON },
  Lexer_Test_Case{ Location{ 84, 0, 0 }, Literal{ (char*) "", 0 }, TK_STRING },
  Lexer_Test_Case{ Location{ 86, 0, 0 }, Literal{ (char*) ";", 1 }, TK_SEMICOLON },
  Lexer_Test_Case{ Location{ 89, 0, 0 }, Literal{ (char*) "var", 3 }, TK_VAR },
  Lexer_Test_Case{ Location{ 93, 0, 0 }, Literal{ (char*) "x", 1 }, TK_IDENT },
  Lexer_Test_Case{ Location{ 95, 0, 0 }, Literal{ (char*) "=", 1 }, TK_ASSIGN },
  Lexer_Test_Case{ Location{ 97, 0, 0 }, Literal{ (char*) "10", 2 }, TK_INTEGER },
  Lexer_Test_Case{ Location{ 99, 0, 0 }, Literal{ (char*) ";", 1 }, TK_SEMICOLON },
  Lexer_Test_Case{ Location{ 101, 0, 0 }, Literal{ (char*) "println", 7 }, TK_PRINTLN },
  Lexer_Test_Case{ Location{ 108, 0, 0 }, Literal{ (char*) "(", 1 }, TK_LPAREN },
  Lexer_Test_Case{ Location{ 109, 0, 0 }, Literal{ (char*) ")", 1 }, TK_RPAREN },
  Lexer_Test_Case{ Location{ 110, 0, 0 }, Literal{ (char*) ";", 1 }, TK_SEMICOLON },
  Lexer_Test_Case{ Location{ 112, 0, 0 }, Literal{ (char*) "", 1 }, TK_EOF },
};

TEST(LexerTestSuite, Lexer_Integration_Test)
{
  Lexer lx;
  lexer_init(&lx, INPUT);

  size_t tc_index = 0;
  Token tk_act;
  Lexer_Test_Case tk_exp;
  do
  {
    tk_act = lexer_next_token(&lx);
    tk_exp = TEST_CASES[tc_index];

    char tk_act_str[128];
    token_to_string(tk_act, tk_act_str, sizeof(tk_act_str));

    EXPECT_EQ(tk_exp.kind, tk_act.kind)
      << "Expected Token_Kind " << token_kind_to_string(tk_exp.kind)
      << ", got " << token_kind_to_string(tk_act.kind);

    // TODO(HS): issues with this with cross platform becaue \r\n - make separate
    // tests covering this functionality
    // EXPECT_EQ(tk_exp.location.pos, tk_act.location.pos);

    // TODO(HS): need to actually do string-wise comparison, otherwise this will
    // always fail
    if (tk_exp.kind != TK_EOF)
    {
      char act_literal_str[128];
      string_view_format_buffer(act_literal_str, sizeof(act_literal_str), tk_act.literal);

      EXPECT_EQ(tk_exp.literal.len, tk_act.literal.len);
      EXPECT_TRUE(string_view_eq_str(tk_act.literal, tk_exp.literal.str))
        << "Expected SV: \"" << tk_exp.literal.str << "\"\n"
        << "Got:         \"" << act_literal_str << "\""
        << "\n"
        << "Got Token:      " << tk_act_str;
    }

    tc_index += 1;
  } while (tk_act.kind != TK_EOF);

  ASSERT_EQ(tc_index, TEST_CASES.size());
}
