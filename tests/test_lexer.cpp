#include <gtest/gtest.h>
#include <vector>
#include <stddef.h>

const char *INPUT = \
  "(){}[];\n"
  "+ - * / !\n"
  "!= == < > <= >=\n"
  "\n"
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

const auto TEST_CASES = std::vector<Token>{
  Token{ Location{ 0, 0, 0 }, String_View{ 0, 1 }, TK_LPAREN  },
  Token{ Location{ 1, 0, 0 }, String_View{ 1, 1 }, TK_RPAREN  },
  Token{ Location{ 2, 0, 0 }, String_View{ 2, 1 }, TK_LBRACE  },
  Token{ Location{ 3, 0, 0 }, String_View{ 3, 1 }, TK_RBRACE  },
  Token{ Location{ 4, 0, 0 }, String_View{ 4, 1 }, TK_LBRACKET  },
  Token{ Location{ 5, 0, 0 }, String_View{ 5, 1 }, TK_RBRACKET  },
  Token{ Location{ 6, 0, 0 }, String_View{ 6, 1 }, TK_SEMICOLON  },
  Token{ Location{ 8, 0, 0 }, String_View{ 8, 1 }, TK_PLUS  },
  Token{ Location{ 10, 0, 0 }, String_View{ 10, 1 }, TK_MINUS  },
  Token{ Location{ 12, 0, 0 }, String_View{ 12, 1 }, TK_ASTERISK  },
  Token{ Location{ 14, 0, 0 }, String_View{ 14, 1 }, TK_SLASH  },
  Token{ Location{ 16, 0, 0 }, String_View{ 16, 1 }, TK_BANG  },
  Token{ Location{ 18, 0, 0 }, String_View{ 18, 2 }, TK_NOT_EQ  },
  Token{ Location{ 21, 0, 0 }, String_View{ 21, 2 }, TK_EQ  },
  Token{ Location{ 24, 0, 0 }, String_View{ 24, 1 }, TK_LT  },
  Token{ Location{ 26, 0, 0 }, String_View{ 26, 1 }, TK_GT  },
  Token{ Location{ 28, 0, 0 }, String_View{ 28, 2 }, TK_LTE  },
  Token{ Location{ 31, 0, 0 }, String_View{ 31, 2 }, TK_GTE  },
  Token{ Location{ 36, 0, 0 }, String_View{ 36, 2 }, TK_INTEGER  },
  Token{ Location{ 38, 0, 0 }, String_View{ 38, 1 }, TK_SEMICOLON  },
  Token{ Location{ 40, 0, 0 }, String_View{ 40, 3 }, TK_INTEGER  },
  Token{ Location{ 43, 0, 0 }, String_View{ 43, 1 }, TK_SEMICOLON  },
  Token{ Location{ 45, 0, 0 }, String_View{ 45, 1 }, TK_INTEGER  },
  Token{ Location{ 46, 0, 0 }, String_View{ 46, 1 }, TK_SEMICOLON  },
  Token{ Location{ 49, 0, 0 }, String_View{ 49, 13 }, TK_STRING  },
  Token{ Location{ 64, 0, 0 }, String_View{ 64, 1 }, TK_SEMICOLON  },
  Token{ Location{ 66, 0, 0 }, String_View{ 66, 19 }, TK_STRING  },
  Token{ Location{ 87, 0, 0 }, String_View{ 87, 1 }, TK_SEMICOLON  },
  Token{ Location{ 89, 0, 0 }, String_View{ 89, 0 }, TK_STRING  },
  Token{ Location{ 91, 0, 0 }, String_View{ 91, 1 }, TK_SEMICOLON  },
  Token{ Location{ 94, 0, 0 }, String_View{ 94, 3 }, TK_VAR  },
  Token{ Location{ 98, 0, 0 }, String_View{ 98, 1 }, TK_IDENT  },
  Token{ Location{ 100, 0, 0 }, String_View{ 100, 1 }, TK_ASSIGN  },
  Token{ Location{ 102, 0, 0 }, String_View{ 102, 2 }, TK_INTEGER  },
  Token{ Location{ 104, 0, 0 }, String_View{ 104, 1 }, TK_SEMICOLON  },
  Token{ Location{ 106, 0, 0 }, String_View{ 106, 7 }, TK_PRINTLN  },
  Token{ Location{ 113, 0, 0 }, String_View{ 113, 1 }, TK_LPAREN  },
  Token{ Location{ 114, 0, 0 }, String_View{ 114, 1 }, TK_RPAREN  },
  Token{ Location{ 115, 0, 0 }, String_View{ 115, 1 }, TK_SEMICOLON  },
  Token{ Location{ 117, 0, 0 }, String_View{ 117, 1 }, TK_EOF  },
};

TEST(LexerTestSuite, Lexer_Integration_Test)
{
  Lexer lx;
  lexer_init(&lx, INPUT);

  size_t tc_index = 0;
  Token  tk_act;
  Token& tk_exp;
  do
  {
    tc_act = lexer_next_token(&lx);
    tc_exp = TEST_CASES[tc_index];

    EXPECT_EQ(tc_exp.kind, tc_act.kind)
      << "Expected Token_Kind " << token_kind_to_string(tc_exp.kind)
      << ", got " << token_kdin_to_string(tc_act.kdin);

    EXPECT_EQ(tc_exp.location.pos, tc_act.location.pos);
    EXPECT_EQ(tc_exp.location.col, tc_act.location.col);
    EXPECT_EQ(tc_exp.location.line, tc_act.location.line);

    EXPECT_EQ(tc_exp.literal.str, tc_act.literal.str);
    EXPECT_EQ(tc_exp.literal.len, tc_act.literal.len);

    tc_index += 1;
  } while (tk_act.kind != TK_EOF);

  ASSERT_EQ(tc_index, TEST_CASES.size());
}
