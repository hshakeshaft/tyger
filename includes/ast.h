#ifndef AST_H_
#define AST_H_
#include <stddef.h>
#include "token.h"

typedef enum error_type
{
#define X(NAME) ERT_ ## NAME,
    #include "defs/parser-error-type.def"
#undef X
    ERROR_TYPE_COUNT
} Error_Type;

typedef enum statement_type
{
#define X(NAME) ST_ ## NAME,
    #include "defs/statement-type.def"
#undef X
    STATEMENT_TYPE_COUNT
} Statement_Type;

typedef enum expression_type
{
#define X(NAME) ET_ ## NAME,
    #include "defs/expression-type.def"
#undef X
    EXPRESSION_TYPE_COUNT
} Expression_Type;


typedef struct error
{
    struct {
        const char *file;
        int line;
        int col;
        int offset;
    } where;
    const char *what;
    Error_Type type;
} Error;

typedef struct statement_handle
{
    unsigned int id;
} Statement_Handle;

typedef struct expression_handle
{
    unsigned int id;
} Expression_Handle;


/* Statements */

typedef struct expression_statement
{
    Expression_Handle handle;
} Expression_Statement;

typedef struct statement
{
    Statement_Type type;
    union {
        Expression_Statement expression;
    } as;
} Statement;


/* Expressions */

typedef struct integer_expression
{
    int value;
} Integer_Expression;

typedef struct expression
{
    Expression_Type type;
    union {
        Integer_Expression integer;
    } as;
} Expression;



/* Program */

typedef struct program
{
    struct {
        Error *elems;
        size_t count;
        size_t capacity;
    } errors;

    /* NOTE(HS): private data */
    struct {
        Statement *elems;
        size_t count;
        size_t capacity;
    } ___statements;

    struct {
        Expression *elems;
        size_t count;
        size_t capacity;
    } ___expressions;
} Program;


#ifdef __cplusplus
extern "C" {
#endif

void program_init(Program *p);

Statement_Handle  program_register_statement(Program *p, Statement *stmt);
Expression_Handle program_register_expression(Program *p, Expression *expr);

const char *ast_statement_type_to_string(Statement_Type type);
const char *ast_expression_type_to_string(Expression_Type type);

void ast__error_create_from_token(Error *error, Error_Type type, Token token);

#ifdef __cplusplus
}
#endif

#endif  /* AST_H_ */
