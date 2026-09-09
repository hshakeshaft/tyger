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
