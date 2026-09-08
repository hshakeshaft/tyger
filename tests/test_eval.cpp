#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "tyger.hpp"

#define ASSERT_OBJECT_TYPE_IS(OBJ, EXPECTED)                             \
    ASSERT_EQ((OBJ)->type, EXPECTED)                                    \
    << "expected object of type " << tyobject_type_to_string(EXPECTED)  \
    << ", got " << tyobject_type_to_string((OBJ)->type)

#define ASSERT_OBJECT_EQ_AS_INT(OBJ, EXPECTED) ASSERT_EQ((OBJ)->as.integer, EXPECTED)

static Program test__parse_program_from_input(const char *input)
{
    Lexer lexer;
    Parser parser;
    Program program;
    lexer_init_from_buffer(&lexer, input);
    parser_init(&parser, &lexer);
    program = parser_parse_program(&parser);
    return program;
}


struct EvalIntTest
{
    std::string input;
    int expected;
};

TEST(EvalTestSuite, Eval_Integer)
{
    auto test_cases = std::vector<EvalIntTest>{
        { "1;", 1 },
        { "100;", 100 },
        { "0;", 0 },
    };

    for (auto& tc : test_cases)
    {
        Program program = test__parse_program_from_input(tc.input.c_str());

        TyVM vm;
        tyvm_init(&vm);
        TyObject *object = eval(&vm, &program);

        ASSERT_OBJECT_TYPE_IS(object, OBJ_INTEGER);
        ASSERT_OBJECT_EQ_AS_INT(object, tc.expected);

        tyvm_deinit(&vm);
    }
}
