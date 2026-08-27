#include <gtest/gtest.h>

#include "token.h"
#include "lexer.h"

#include <string>
#include <vector>

struct Lexical_Token_Test
{
    std::string input;
    Token_Type expected_type;
};

TEST(LexerTestSuite, test_lexer_lexes_tokens)
{
    auto test_cases = std::vector<Lexical_Token_Test>{
        { "\0",                TT_EOF       },
        { ";" ,                TT_SEMICOLON },
        { "(" ,                TT_LPAREN    },
        { ")" ,                TT_RPAREN    },
        { "{" ,                TT_LBRACE    },
        { "}" ,                TT_RBRACE    },
        { "[" ,                TT_LBRACKET  },
        { "]" ,                TT_RBRACKET  },
        { "<" ,                TT_LT        },
        { ">" ,                TT_GT        },
        { "!" ,                TT_BANG      },
        { "=" ,                TT_ASSIGN    },
        { "+" ,                TT_ADD       },
        { "-" ,                TT_SUB       },
        { "*" ,                TT_MUL       },
        { "/" ,                TT_DIV       },
        { "==",                TT_EQ        },
        { "!=",                TT_NEQ       },
        { "<=",                TT_LTE       },
        { ">=",                TT_GTE       },
        #if 0
        { "var",               TT_KW_VAR    },
        { "\"foo\"",           TT_STRING    },
        { "\"Hello, World!\"", TT_STRING    },
        { "10",                TT_INT       },
        { "51013",             TT_INT       },
        { "println",           TT_IDENT     },
        { "x",                 TT_IDENT     },
        { "y",                 TT_IDENT     },
        { "foo_bar",           TT_IDENT     },
        { "fooBar",            TT_IDENT     },
        #endif
    };

    for (auto& tc : test_cases)
    {
        Lexer lexer;
        ASSERT_TRUE(lexer_init_from_buffer(&lexer, tc.input.c_str()));

        Token token = lexer_next_token(&lexer);
        ASSERT_EQ(token.type, tc.expected_type) 
            << "exepected token type " << token_type_to_string(tc.expected_type)
            << ", got " << token_type_to_string(token.type);
        // TODO(HS): assert literal is expected
    }
}

#if 0
TEST(LexerTestSuite, test_lexer_reports_position)
{
    // NOTE(HS): test that all line encoding styles are handled as line increments
    // NOTE(HS): `\r` is legacy MacOS, unlikely anything will ever hit this, included
    // for completeness
    auto input = std::string{
        "10;\\n"
        "5;\\r"
        "1;\\r\\n"
        "0;"
    };

    Lexer lexer;
    lexer_init_from_buffer(&lexer, input.c_str());

    Token token;

    auto test_token_location = [&token] (std::string file, int line, int col, int offset) {
        ASSERT_EQ(std::string{token.file}, file);
        ASSERT_EQ(token.line, line);
        ASSERT_EQ(token.col, col);
        ASSERT_EQ(token.offset, offset);
    };

    token = lexer_next_token(&lexer);  // `10`
    test_token_location("<NULL>", 1, 1, 0);
    token = lexer_next_token(&lexer);  // `;`
    test_token_location("<NULL>", 1, 3, 2);
    lexer_next_token(&lexer);  // `\n`

    token = lexer_next_token(&lexer);  // `5`
    test_token_location("<NULL>", 2, 1, 4);
    lexer_next_token(&lexer);  // `;`
    lexer_next_token(&lexer);  // `\r`

    token = lexer_next_token(&lexer);  // `1`
    test_token_location("<NULL>", 3, 1, 7);
    test_lexer_next_token(&lexer);  // `;`
    test_lexer_next_token(&lexer);  // `\r\n`

    token = lexer_next_token(&lexer);  // `0`
    test_token_location("<NULL>", 4, 1, 11);
    token = lexer_next_token(&lexer);  // `0`
    test_token_location("<NULL>", 4, 2, 12);
}
#endif
