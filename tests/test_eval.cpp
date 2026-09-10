#include <gtest/gtest.h>

#include "helpers/helpers.hpp"


struct EvalIntTest
{
    std::string input;
    int expected;
};

struct EvalStringTest
{
    std::string input;
    std::string expected;
};

struct EvalVarStatementTest
{
    std::string input;
    TyObject_Type expected_object_type;
    std::string expected_ident;
    int expected_integer;
    std::string expected_string;
};

// TODO(HS): move the internal mangling where I check for correct object registration
// from insertion of ident into another test (keep in eval for now then maybe move
// to vm only tests?)
TEST(EvalTestSuite, Eval_Var_Statement)
{
    auto test_cases = std::vector<EvalVarStatementTest>{
        { "var x = 100;",          OBJ_INTEGER, "x",   100, {} },
        { "var foo = \"Henlo!\";", OBJ_STRING,  "foo", {},  "Henlo!" }
    };

    for (auto& tc : test_cases)
    {
        Program program = test__parse_program_from_input(tc.input.c_str());

        TyVM vm;
        tyvm_init(&vm);

        TyObject *object = eval(&vm, &program);

        ASSERT_OBJECT_TYPE_IS(object, OBJ_NONE);

        object = vm_get_ident(&vm, tc.expected_ident.c_str());
        ASSERT_NE(object, nullptr);
        ASSERT_OBJECT_TYPE_IS(object, OBJ_IDENT);

        object = object->as.ident.value;
        ASSERT_OBJECT_TYPE_IS(object, tc.expected_object_type);

        switch (object->type)
        {
            case OBJ_INTEGER: {
                ASSERT_EQ(object->as.integer, tc.expected_integer);
            } break;

            case OBJ_STRING: {
                ASSERT_EQ(
                    std::string(object->as.string.str, object->as.string.len),
                    tc.expected_string
                );
            } break;

            default: {
                FAIL();
            }
        }

        tyvm_deinit(&vm);
    }
}

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

TEST(EvalTestSuite, Eval_Strings)
{
    auto test_cases = std::vector<EvalStringTest>{
        { "\"Henlo!\";", "Henlo!" },
        { "\"Hello, World!\";", "Hello, World!" },
        { "\"the quick brown fox jumps over the lazy dog\";", "the quick brown fox jumps over the lazy dog" },
    };

    for (auto& tc : test_cases)
    {
        Program program = test__parse_program_from_input(tc.input.c_str());

        TyVM vm;
        tyvm_init(&vm);
        TyObject *object = eval(&vm, &program);

        ASSERT_OBJECT_TYPE_IS(object, OBJ_STRING);
        ASSERT_OBJECT_EQ_AS_STRINGS(object, tc.expected);

        tyvm_deinit(&vm);
    }
}
