#ifndef TYGER_LEXER_INTERNAL_H_
#define TYGER_LEXER_INTERNAL_H_
#include <stdbool.h>
#include "lexer.h"

#define make_location(POS, COL, LINE) (Location) { .pos = (POS), .col = (COL), .line = (LINE) }

void lexer_read_char(Lexer *lx);
char lexer_peek_char(Lexer *lx);
void lexer_skip_whitespace(Lexer *lx);

void lexer_read_number(Lexer *lx);
void lexer_read_string(Lexer *lx);
void lexer_read_ident_or_keyword(Lexer *lx);

Token_Kind string_view_to_token_kind(String_View sv);

bool is_whitespace(char c);
bool is_digit(char c);
bool is_char(char c);

#endif // TYGER_LEXER_INTERNAL_H_
