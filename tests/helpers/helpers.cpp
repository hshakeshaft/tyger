#include "helpers.hpp"

////////////////////////////////////////////////////////////////////////////////
/// Lexer Test Helpers

std::ostream& operator<<(std::ostream& strm, const Lexer& l)
{
    strm
        << "Lexer{\n"
        << "  .input = \n```\n" << l.input << "\n```, \n"
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


////////////////////////////////////////////////////////////////////////////////
/// Parser Test Helpers

void parser__check_errors_and_log(Program program)
{
    if (program.errors.count != 0)
    {
        for (size_t i = 0; i < program.errors.count; ++i)
        {
            Error *cur_err;
            cur_err = &program.errors.elems[i];
            std::cerr << "ERROR :: " << cur_err->what << '\n';
        }
        ASSERT_EQ(program.errors.count, 0);
    }
}

std::string program__expression_to_parse_tree(Program program, Expression *expr)
{
    std::string result{};

    switch (expr->type)
    {
        case ET_INTEGER: {
            result += std::to_string(expr->as.integer.value);
        } break;

        case ET_STRING: {
            result += std::string(expr->as.string.ptr);
        } break;

        case ET_IDENT: {
            result += std::string(expr->as.ident.name);
        } break;

        case ET_INFIX: {
            result += "(";

            Infix_Expression *infix = &expr->as.infix;
            switch (infix->op)
            {
                case OP_ADD: result += "+ "; break;
                case OP_SUB: result += "- "; break;
                case OP_MUL: result += "* "; break;
                case OP_DIV: result += "/ "; break;
                default:;
            }

            Expression *lhs = program_expression_handle_to_expression(&program, infix->lhs);
            Expression *rhs = program_expression_handle_to_expression(&program, infix->rhs);

            result += program__expression_to_parse_tree(program, lhs);
            result += " ";
            result += program__expression_to_parse_tree(program, rhs);

            result += ")";
        } break;

        default:;
    }

    return result;
}

std::string program__statement_to_parse_tree(Program program, Statement *stmt)
{
    std::string result{};

    switch (stmt->type)
    {
        case ST_VAR: {
            Expression *expr = program_expression_handle_to_expression(&program, stmt->as.var.expression);
            auto expr_str = program__expression_to_parse_tree(program, expr);

            result += "(var ";
            result += std::string{stmt->as.var.ident};
            result += " ";
            result += expr_str;
            result += ")";
        } break;

        case ST_EXPRESSION: {
            Expression *expr = program_expression_handle_to_expression(&program, stmt->as.expression.handle);
            auto expr_str = program__expression_to_parse_tree(program, expr);
            result += expr_str;
        } break;

        default:;
    }

    return result;
}

std::string program__to_parse_tree(Program program)
{
    std::string result{};

    for (size_t i = 0; i < program.___statements.count; ++i)
    {
        auto stmt = &program.___statements.elems[i];
        switch (stmt->type)
        {
            case ST_EXPRESSION: {
                Expression *expr = program_expression_handle_to_expression(&program, stmt->as.expression.handle);
                result = program__expression_to_parse_tree(program, expr);
            } break;

            default:;
        }
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////
/// Eval Test Helpers

Program test__parse_program_from_input(const char *input)
{
    Lexer lexer;
    Parser parser;
    Program program;
    lexer_init_from_buffer(&lexer, input);
    parser_init(&parser, &lexer);
    program = parser_parse_program(&parser);
    return program;
}
