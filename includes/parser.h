#ifndef PARSER_H_
#define PARSER_H_
#include "ast.h"
#include "lexer.h"

typedef struct parser
{
    Lexer *lx;
    Token cur_token;
    Token peek_token;
} Parser;


#ifdef __cplusplus
extern "C" {
#endif

void parser_init(Parser *ps, Lexer *lx);

Program parser_parse_program(Parser *ps);

#ifdef __cplusplus
}
#endif

#endif  /* PARSER_H_ */
