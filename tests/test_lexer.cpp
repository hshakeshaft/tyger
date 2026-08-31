#include <gtest/gtest.h>

#include "token.h"
#include "lexer.h"

#include <string>
#include <vector>
#include <ostream>

struct Lexical_Token_Test
{
    std::string input;
    Token_Type expected_type;
    const char *expected_literal;
};

std::ostream& operator<<(std::ostream& strm, const Lexer& l)
{
    strm
        << "Lexer{\n"
        << "  .input = \"" << l.input << "\", \n"
        << "  .pos = " << l.pos << ",\n"
        << "  .read_pos = " << l.read_pos << ",\n"
        << "  .ch = '" << l.ch << "',\n"
        << "  .file = \"" << (l.file ? l.file : "<NULL>") << "\",\n"
        << "  .line = " << l.line << ",\n"
        << "  .col = " << l.col << "\n"
        << "}"
    ;
    return strm;
}

std::ostream& operator<<(std::ostream& strm, const Token& t)
{
    strm 
        << "Token{\n"
        << "  .type = " << token_type_to_string(t.type) << ",\n"
        << "  .literal = \"" << std::string(t.literal.str, t.literal.len) << "\",\n"
        << "  .file = \"" << (t.file ? t.file : "<NULL>") << "\",\n"
        << "  .line = " << t.line << ",\n"
        << "  .col = " << t.col << ",\n"
        << "  .offset = " << t.offset << "\n"
        << "}"
    ;
    return strm;
}

TEST(LexerTestSuite, test_lexer_lexes_tokens)
{
    auto test_cases = std::vector<Lexical_Token_Test>{
        { "\0",                TT_EOF,       "\0"  },
        { ";" ,                TT_SEMICOLON, ";",  },
        { "(" ,                TT_LPAREN,    "("   },
        { ")" ,                TT_RPAREN,    ")"   },
        { "{" ,                TT_LBRACE,    "{"   },
        { "}" ,                TT_RBRACE,    "}"   },
        { "[" ,                TT_LBRACKET,  "["   },
        { "]" ,                TT_RBRACKET,  "]"   },
        { "<" ,                TT_LT,        "<"   },
        { ">" ,                TT_GT,        ">"   },
        { "!" ,                TT_BANG,      "!"   },
        { "=" ,                TT_ASSIGN,    "="   },
        { "+" ,                TT_ADD,       "+"   },
        { "-" ,                TT_SUB,       "-"   },
        { "*" ,                TT_MUL,       "*"   },
        { "/" ,                TT_DIV,       "/"   },
        { "==",                TT_EQ,        "=="  },
        { "!=",                TT_NEQ,       "!="  },
        { "<=",                TT_LTE,       "<="  },
        { ">=",                TT_GTE,       ">="  },

        { "var",               TT_KW_VAR, "var"     },
        { "x",                 TT_IDENT,  "x"       },
        { "y",                 TT_IDENT,  "y"       },
        { "foo_bar",           TT_IDENT,  "foo_bar" },
        { "fooBar",            TT_IDENT,  "fooBar"  },
        { "println",           TT_IDENT,  "println" },

        { "10",                TT_INT, "10"    },
        { "51013",             TT_INT, "51013" },

        { "\"foo\"",           TT_STRING, "foo"           },
        { "\"Hello, World!\"", TT_STRING, "Hello, World!" },
    };

    for (auto& tc : test_cases)
    {
        Lexer lexer;
        ASSERT_TRUE(lexer_init_from_buffer(&lexer, tc.input.c_str()));

        Token token = lexer_next_token(&lexer);

        ASSERT_EQ(token.type, tc.expected_type) 
            << "exepected token type " << token_type_to_string(tc.expected_type)
            << ", got " << token
            << "\n\nLexer State\n" << lexer;

        // NOTE(HS): had some issues asserting the literal was `\0` - in reality
        // this will never be checked as processing will terminate on reading type
        if (token.type != TT_EOF) [[likely]]
        {
            ASSERT_EQ(
                std::string(token.literal.str, token.literal.len),
                std::string(tc.expected_literal)
            );
        }
    }
}

TEST(LexerTestSuite, test_lexer_reports_position)
{
    // NOTE(HS): test that all line encoding styles are handled as line increments
    // NOTE(HS): `\r` is legacy MacOS, unlikely anything will ever hit this, included
    // for completeness
    auto input = std::string{
        "var x;\n"
        "x = 10;\r"
        "x = 5;\r\n"
        "x = 1;"
    };

    Lexer lexer;
    lexer_init_from_buffer(&lexer, input.c_str());

    Token token;

    auto test_token_location = [&token, lexer] (std::string file, int line, int col, int offset) {
        ASSERT_EQ(std::string{token.file ? token.file : "<NULL>"}, file) << token << "\n" << lexer << "\n";

        ASSERT_EQ(token.line, line)     << "Token:\n" << token << "\n" << lexer << "\n";
        ASSERT_EQ(token.col, col)       << "Token:\n" << token << "\n" << lexer << "\n";
        ASSERT_EQ(token.offset, offset) << "Token:\n" << token << "\n" << lexer << "\n";
    };

    // NOTE(HS): this is effectively obbsessive checking for all tokens on the first
    // line
    token = lexer_next_token(&lexer);        // `var`
    test_token_location("<NULL>", 1, 1, 0);
    token = lexer_next_token(&lexer);        // `x`
    test_token_location("<NULL>", 1, 5, 4);
    token = lexer_next_token(&lexer);        // `;`
    test_token_location("<NULL>", 1, 6, 5);

    token = lexer_next_token(&lexer);       // `x`
    test_token_location("<NULL>", 2, 1, 7);
    token = lexer_next_token(&lexer);       // `=`
    token = lexer_next_token(&lexer);       // `10`
    token = lexer_next_token(&lexer);       // `;`

    token = lexer_next_token(&lexer);        // `x`
    test_token_location("<NULL>", 3, 1, 15);
    token = lexer_next_token(&lexer);        // `=`
    token = lexer_next_token(&lexer);        // `5`
    token = lexer_next_token(&lexer);        // `;`

    token = lexer_next_token(&lexer);        // `x`
    test_token_location("<NULL>", 4, 1, 23);
}
