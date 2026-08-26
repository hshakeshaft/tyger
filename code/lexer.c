#include <stdio.h>
#include <string.h>

#include "lexer.h"

static void lexer__next_char(Lexer *lx)
{
    if (lx->read_pos >= lx->input_len)
    {
        lx->ch = '\0';
    }
    else
    {
        lx->ch = lx->input[lx->read_pos];
    }
    lx->pos = lx->read_pos++;
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

    lexer__next_char(lexer);

    success = 1;

    return success;
}

Token lexer_next_token(Lexer *lx)
{
    Token token;

    token.type = TT_ILLEGAL;

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
            token.type = TT_LT;
        } break;

        case '>': {
            token.type = TT_GT;
        } break;

        case '!': {
            token.type = TT_BANG;
        } break;

        case '=': {
            token.type = TT_ASSIGN;
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
    }

    lexer__next_char(lx);

    return token;
}
