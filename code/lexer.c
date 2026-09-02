#include <stdio.h>
#include <string.h>

#include "lexer.h"

static int lexer__peek_char_is(Lexer *lx, char ch)
{
    if (lx->read_pos >= lx->input_len)
    {
        return 0;
    }
    else
    {
        return lx->input[lx->read_pos] == ch;
    }
}

static void lexer__next_char(Lexer *lx)
{
    if (lx->read_pos >= lx->input_len)
    {
        lx->ch = '\0';
        lx->pos = lx->read_pos++;
        return;
    }
    else
    {
        if (lx->ch == '\n')
        {
            lx->col = 1;
            lx->line++;
        }
        else if (lx->ch == '\r')
        {
            if (lexer__peek_char_is(lx, '\n'))
            {
                lx->read_pos++;
            }
            lx->col = 1;
            lx->line++;
        }
        else
        {
            lx->col++;
        }

        lx->ch = lx->input[lx->read_pos];
    }
    lx->pos = lx->read_pos++;
}

static int lexer__ch_is_numeric(char ch)
{
    int istrue;
    istrue = '0' <= ch && ch <= '9';
    return istrue;
}

static int lexer__ch_is_space(char ch)
{
    int istrue;
    istrue = ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r';
    return istrue;
}

static int lexer__ch_is_symbol(char ch)
{
    #define SYMBOL_COUNT 15
    static char symbols[SYMBOL_COUNT] = {
        '(', ')', '{', '}', '[', ']',
        ';', '<', '>', '!', '=', '+', '-', '*', '/'
    };
    int istrue;
    int i;
    istrue = 0;
    for (i = 0; i < SYMBOL_COUNT; ++i)
    {
        if (ch == symbols[i])
        {
            istrue = 1;
            break;
        }
    }
    return istrue;
}

static void lexer__skip_whitespace(Lexer *lx)
{
    while (lexer__ch_is_space(lx->ch))
    {
        lexer__next_char(lx);
    }
}

static Token_Type lexer__sv_to_token_type(String_View sv)
{
    #define LEXER__SV_CMP_CSTR(SV, CSTR)            \
        ( ( (sizeof(CSTR) - 1) == (SV).len )        \
        && strncmp((SV).str, (CSTR), (SV).len) == 0 )

    if (LEXER__SV_CMP_CSTR(sv, "var")) { return TT_KW_VAR; }
    else { return TT_IDENT; }
}

static String_View lexer__read_ident_or_keyword(Lexer *lx, Token_Type *type)
{
    String_View literal;
    int pos;
    int lit_len;

    pos = lx->pos;

    while (!lexer__ch_is_space(lx->ch) && !lexer__ch_is_symbol(lx->ch) && lx->ch != '\0')
    {
        lexer__next_char(lx);
    }

    lit_len = lx->pos - pos;
    literal = sv_from_cstring(&lx->input[pos], lit_len);

    *type = lexer__sv_to_token_type(literal);

    return literal;
}

static String_View lexer__read_number(Lexer *lx)
{
    String_View literal;
    int pos;
    int lit_len;

    pos = lx->pos;

    while (!lexer__ch_is_space(lx->ch) && lexer__ch_is_numeric(lx->ch) && lx->ch != '\0')
    {
        lexer__next_char(lx);
    }

    lit_len = lx->pos - pos;
    literal = sv_from_cstring(&lx->input[pos], lit_len);

    return literal;
}

static String_View lexer__read_string(Lexer *lx)
{
    String_View literal;
    int pos;
    int lit_len;

    lexer__next_char(lx);

    pos = lx->pos;
    while (lx->ch != '\"' && lx->ch != '\0')
    {
        if (lx->ch == '\\' && lexer__peek_char_is(lx, '\"'))
        {
            lexer__next_char(lx);
        }
        lexer__next_char(lx);
    }
    lit_len = lx->pos - pos;
    literal = sv_from_cstring(&lx->input[pos], lit_len);

    return literal;
}


int lexer_init_from_buffer(Lexer *lexer, const char *input_buffer)
{
    int success;
    int input_len;

    success = 0;

    if (!input_buffer)
    {
        fprintf(stderr, "Invalid address for buffer provided (%p)\n", input_buffer);
        return success;
    }

    input_len = strlen(input_buffer);
    if (input_len < 0) { return success; }

    lexer->input = input_buffer;
    lexer->input_len = input_len;
    lexer->pos = 0;
    lexer->read_pos = 0;
    lexer->ch = 0;

    lexer->file = NULL;
    lexer->line = 1;
    lexer->col = 0;

    lexer__next_char(lexer);

    success = 1;

    return success;
}

Token lexer_next_token(Lexer *lx)
{
    Token token;

    lexer__skip_whitespace(lx);

    token.type    = TT_ILLEGAL;
    token.literal = sv_from_cstring(&lx->input[lx->pos], 1);
    token.file    = NULL;
    token.line    = lx->line;
    token.col     = lx->col;
    token.offset  = lx->pos;

    switch (lx->ch)
    {
        case '\0': {
            token.type = TT_EOF;
        } break;

        case ';': {
            token.type = TT_SEMICOLON;
        } break;

        case '(': {
            token.type = TT_LPAREN;
        } break;

        case ')': {
            token.type = TT_RPAREN;
        } break;

        case '{': {
            token.type = TT_LBRACE;
        } break;

        case '}': {
            token.type = TT_RBRACE;
        } break;

        case '[': {
            token.type = TT_LBRACKET;
        } break;

        case ']': {
            token.type = TT_RBRACKET;
        } break;

        case '<': {
            if (lexer__peek_char_is(lx, '='))
            {
                token.type = TT_LTE;
                token.literal.len += 1;
                lexer__next_char(lx);
            }
            else
            {
                token.type = TT_LT;
            }
        } break;

        case '>': {
            if (lexer__peek_char_is(lx, '='))
            {
                token.type = TT_GTE;
                token.literal.len += 1;
                lexer__next_char(lx);
            }
            else
            {
                token.type = TT_GT;
            }
        } break;

        case '!': {
            if (lexer__peek_char_is(lx, '='))
            {
                token.type = TT_NEQ;
                token.literal.len += 1;
                lexer__next_char(lx);
            }
            else
            {
                token.type = TT_BANG;
            }
        } break;

        case '=': {
            if (lexer__peek_char_is(lx, '='))
            {
                token.type = TT_EQ;
                token.literal.len += 1;
                lexer__next_char(lx);
            }
            else
            {
                token.type = TT_ASSIGN;
            }
        } break;

        case '+': {
            token.type = TT_ADD;
        } break;

        case '-': {
            token.type = TT_SUB;
        } break;

        case '*': {
            token.type = TT_MUL;
        } break;

        case '/': {
            token.type = TT_DIV;
        } break;

        default: {
            if (lexer__ch_is_numeric(lx->ch))
            {
                String_View literal;
                literal = lexer__read_number(lx);
                token.type = TT_INT;
                token.literal = literal;
                return token;
            }
            else if (lx->ch == '\"')
            {
                String_View literal;
                literal = lexer__read_string(lx);

                /* NOTE(HS): if after reading a string, the string is unterminated
                then this should result in an illegal token being generated */
                if (lx->ch == '\"')
                {
                    lexer__next_char(lx);
                    token.type = TT_STRING;
                }
                else
                {
                    token.type = TT_ILLEGAL;
                }

                token.literal = literal;
                return token;
            }
            else
            {
                String_View literal;
                Token_Type type;
                literal = lexer__read_ident_or_keyword(lx, &type);
                token.literal = literal;
                token.type = type;
                return token;
            }
        }
    }

    lexer__next_char(lx);

    return token;
}
