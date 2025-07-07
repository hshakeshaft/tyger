#ifndef TYGER_PARSER_H_
#define TYGER_PARSER_H_
#include <stdint.h>
#include <stddef.h>
#include "tstrings.h"
#include "lexer.h"

typedef size_t Ident_Handle;
typedef size_t Expression_Handle;

typedef enum tyger_error_kind
{
#define X(NAME) TYERR_##NAME,
  #include "defs/tyger-error-kind.def"
#undef X
} Tyger_Error_Kind;

typedef enum statement_kind
{
#define X(NAME) STMT_##NAME,
  #include "defs/statement-kind.def"
#undef X
} Statement_Kind;

typedef enum expression_kind
{
#define X(NAME) EXPR_##NAME,
  #include "defs/expression-kind.def"
#undef X
} Expression_Kind;

typedef enum
{
#define X(NAME, ...) OP_##NAME,
  #include "defs/operator.def"
#undef X
} Operator;

typedef struct tyger_error
{
  Tyger_Error_Kind kind;
} Tyger_Error;

typedef struct expression Expression;

typedef struct int_expression
{
  int64_t value;
} Int_Expression;

typedef struct string_expression
{
  const char *value;
  size_t len;
} String_Expression;

typedef struct ident_expression
{
  const char *ident;
} Ident_Expression;

typedef struct infix_expression
{
  Operator op;
  Expression *lhs;
  Expression *rhs;
} Infix_Expression;

typedef struct argument_list
{
  Expression *elems;
  size_t capacity;
  size_t len;
} Argument_List;

typedef struct call_expression
{
  Expression *function;
  Argument_List args;
} Call_Expression;

typedef union uexpression
{
  Int_Expression int_expression;
  String_Expression string_expression;
  Ident_Expression ident_expression;
  Infix_Expression infix_expression;
  Call_Expression call_expression;
} uExpression;

struct expression
{
  Expression_Kind kind;
  uExpression expression;
};

typedef struct var_statement
{
  Ident_Handle ident_handle;
  Expression_Handle expression_handle;
} Var_Statement;

typedef struct expression_statement
{
  Expression_Handle expression_handle;
} Expression_Statement;

typedef union ustatement
{
  Var_Statement var_statement;
  Expression_Statement expression_statement;
} uStatement;

typedef struct statement
{
  Statement_Kind kind;
  uStatement statement;
} Statement;

typedef struct statement_vaarray
{
  Statement *elems;
  size_t capacity;
  size_t len;
} Statement_VaArray;

typedef struct error_vaarray
{
  Tyger_Error *elems;
  size_t capacity;
  size_t len;
} Error_VaArray;

typedef struct string_vaarray
{
  char *elems;
  size_t capacity;
  size_t len;
} String_VaArray;

typedef struct expression_vaarray
{
  Expression *elems;
  size_t capacity;
  size_t len;
} Expression_VaArray;

// TODO(HS): use a hash map for identifiers?
typedef struct parser_context
{
  String_VaArray identifiers;
  String_VaArray evaluated_identifiers;
  Expression_VaArray expressions;
  String_VaArray strings;
} Parser_Context;

typedef struct parser
{
  Lexer *lexer;
  Token cur_token;
  Token peek_token;
} Parser;

typedef struct program
{
  Statement_VaArray statements;
  Error_VaArray errors;
  Parser_Context context;
} Program;

void parser_init(Parser *p, Lexer *lx);
Program parser_parse_program(Parser *p);
void program_free(Program *p);

const char *tyger_error_kind_to_string(Tyger_Error_Kind kind);
const char *statement_kind_to_string(Statement_Kind kind);
const char *expression_kind_to_string(Expression_Kind kind);
const char *operator_to_string(Operator op);

const char *ident_handle_to_ident(const Program *p, Ident_Handle hndl);
const Expression *expression_handle_to_expression(const Program *p, Expression_Handle hndl);

Tyger_Error parser_parse_statement(Parser *p, Parser_Context *ctx, Statement *stmt);
Tyger_Error parse_var_statement(Parser *p, Parser_Context *ctx, Statement *stmt);
Tyger_Error parse_expression_statement(Parser *p, Parser_Context *ctx, Statement *stmt);

Tyger_Error parse_int_expression(Parser *p, Expression *expr);
Tyger_Error parse_string_expression(Parser *p, Parser_Context *ctx, Expression *expr);
Tyger_Error parse_ident_expression(Parser *p, Parser_Context *ctx, Expression *expr);
Tyger_Error parse_infix_expression(Parser *p, Parser_Context *ctx, Expression *lhs);
Tyger_Error parse_call_expression(Parser *p, Parser_Context *ctx, Expression *expr);
Tyger_Error parse_call_expression_args(Parser *p, Parser_Context *ctx, Argument_List *args);

#endif // TYGER_PARSER_H_
