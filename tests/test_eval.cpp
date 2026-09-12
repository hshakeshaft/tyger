#include <gtest/gtest.h>

#include "helpers/helpers.hpp"

union ExpectedObjectValue
{
    int         as_integer;
    const char *as_string;
};

struct EvalVarStatementTest
{
    std::string input;
    TyObject_Type expected_object_type;
    std::string expected_ident;
    ExpectedObjectValue expected;
};

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

struct EvalIdentTest
{
    std::string input;
    std::string ident;
    TyObject_Type expected_object_type;
    ExpectedObjectValue expected;
};

struct EvalInfixTest
{
    std::string input;
    int expected;
};


// TODO(HS): move the internal mangling where I check for correct object registration
// from insertion of ident into another test (keep in eval for now then maybe move
// to vm only tests?)
TEST_F(EvalTestFixture, Eval_Var_Statement)
{
    auto test_cases = std::vector<EvalVarStatementTest>{
        { "var x = 100;",          OBJ_INTEGER, "x",   { .as_integer = 100 } },
        { "var foo = \"Henlo!\";", OBJ_STRING,  "foo", { .as_string = "Henlo!" } },
    };

    for (auto& tc : test_cases)
    {
        this->init(tc.input.c_str());
        auto *object = eval(&this->vm, &this->m_program);
        ASSERT_OBJECT_TYPE_IS(object, OBJ_NONE);

        object = vm_get_ident(&this->vm, tc.expected_ident.c_str());
        ASSERT_NE(object, nullptr);
        ASSERT_OBJECT_TYPE_IS(object, OBJ_IDENT);

        object = object->as.ident.value;
        ASSERT_NE(object, nullptr);
        ASSERT_OBJECT_TYPE_IS(object, tc.expected_object_type);
        ASSERT_OBJECT_IS_EQUAL_TO(object, tc.expected);

        this->reset();
    }
}

TEST_F(EvalTestFixture, Eval_Integer)
{
    auto test_cases = std::vector<EvalIntTest>{
        { "1;", 1 },
        { "100;", 100 },
        { "0;", 0 },
    };

    for (auto& tc : test_cases)
    {
        this->init(tc.input.c_str());
        auto *object = eval(&this->vm, &this->m_program);
        ASSERT_NE(object, nullptr);
        ASSERT_OBJECT_TYPE_IS(object, OBJ_INTEGER);
        ASSERT_OBJECT_EQ_AS_INT(object, tc.expected);
        this->reset();
    }
}

TEST_F(EvalTestFixture, Eval_String)
{
    auto test_cases = std::vector<EvalStringTest>{
        { "\"Henlo!\";", "Henlo!" },
        { "\"Hello, World!\";", "Hello, World!" },
        { "\"the quick brown fox jumps over the lazy dog\";", "the quick brown fox jumps over the lazy dog" },
    };

    for (auto& tc : test_cases)
    {
        this->init(tc.input.c_str());
        auto *object = eval(&this->vm, &this->m_program);
        ASSERT_NE(object, nullptr);
        ASSERT_OBJECT_TYPE_IS(object, OBJ_STRING);
        ASSERT_OBJECT_EQ_AS_STRINGS(object, tc.expected);
        this->reset();
    }
}

TEST_F(EvalTestFixture, Eval_Ident)
{
    auto test_cases = std::vector<EvalIdentTest>{
        { "var x = 10; x;",           "x", OBJ_INTEGER, { .as_integer = 10 } },
        { "var y = \"Hellope!\"; y;", "y", OBJ_STRING,  { .as_string = "Hellope!" } }
    };

    for (auto& tc : test_cases)
    {
        this->init(tc.input.c_str());
        auto *object = eval(&this->vm, &this->m_program);
        ASSERT_NE(object, nullptr);
        ASSERT_OBJECT_TYPE_IS(object, OBJ_IDENT);
        ASSERT_EQ(object->as.ident.ident, tc.ident);

        object = object->as.ident.value;
        ASSERT_NE(object, nullptr);
        ASSERT_OBJECT_TYPE_IS(object, tc.expected_object_type);
        ASSERT_OBJECT_IS_EQUAL_TO(object, tc.expected);

        this->reset();
    }
}

TEST_F(EvalTestFixture, Eval_Infix_Expression)
{
    auto test_cases = std::vector<EvalInfixTest>{
        { "2 + 2;", 4 },
        { "2 - 2;", 0 },
        { "0 - 2;", -2 },
        { "2 * 3;", 6 },
        { "10 / 2;", 5 },
        { "1 + 2 + 3 + 4 + 5;", 15 },
        { "1 * 2 * 3 * 4 * 5;", 120 },
    };

    for (auto& tc : test_cases)
    {
        this->init(tc.input.c_str());

        auto *object = eval(&this->vm, &this->m_program);
        ASSERT_NE(object, nullptr);
        ASSERT_OBJECT_TYPE_IS(object, OBJ_INTEGER);
        ASSERT_OBJECT_EQ_AS_INT(object, tc.expected);

        this->reset();
    }
}
