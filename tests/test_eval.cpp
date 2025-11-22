#include <gtest/gtest.h>
#include <cstdint>
#include <vector>
#include <string>
#include "tyger_test.hpp"
#include "../tests/parser_test_helper.hpp"

///
/// Expression Eval Tests
///

TEST(EvalTestSuite, Test_Eval_Int_Expression)
{
  struct Int_Eval_TC {
    const char *input;
    int64_t exp;
  };

  auto test_cases = std::vector<Int_Eval_TC>{
    { "5;", 5 },
    { "100;", 100 },
    { "0;", 0 },
    { "314159;", 314159 },
  };

  for (auto& tc : test_cases)
  {
    SETUP_PARSER_TEST_CASE(tc.input);
    DEFER({ program_free((Program*) &p); });
    TyEnv env;
    tyenv_init(&env, 16);
    TyObj val = eval(&env, &p);
    ASSERT_EQ(val.kind, TYOBJ_INT)
      << "Expected object to be " << tyobj_kind_to_string(TYOBJ_INT)
      << " (integer), got " << tyobj_kind_to_string(val.kind);
    ASSERT_EQ(val.o.integer.value, tc.exp);
  }
}

TEST(EvalTestSuite, Test_Eval_String_Expression)
{
  struct String_Eval_Tc {
    const char *input;
    const char *exp;
    size_t exp_len;
  };

  auto test_cases = std::vector<String_Eval_Tc>{
    { "\"Hello\";", "Hello", 5 },
    { "\"\";", "", 0 },
    {
      "\"The Quick Brown Fox Jumps Over The Lazy Dog\";",
      "The Quick Brown Fox Jumps Over The Lazy Dog", 43
    },
  };

  for (auto& tc : test_cases)
  {
    SETUP_PARSER_TEST_CASE(tc.input);
    DEFER({ program_free((Program*) &p); });
    TyEnv env;
    tyenv_init(&env, 16);
    TyObj val = eval(&env, &p);
    ASSERT_EQ(val.kind, TYOBJ_STRING)
      << "Expected object to be " << tyobj_kind_to_string(TYOBJ_STRING)
      << " (string), got " << tyobj_kind_to_string(val.kind);
    auto exp_s = std::string{tc.exp};
    auto act_s = std::string{val.o.string.value};
    ASSERT_EQ(exp_s, act_s);
  }
}

TEST(EvalTestSuite, Test_Eval_Infix_Expression)
{
  struct Infix_Eval_Int_Tc {
    const char *input;
    int64_t exp;
  };

  auto test_cases = std::vector<Infix_Eval_Int_Tc>{
    { "1 + 1;", 2 },
    { "1 - 1;", 0 },
    { "4 / 2;", 2 },
    { "2 * 2;", 4 },
    { "2 * 2 * 2;", 8 },
    { "5 * 4 + 3 - 2 / 1;", 21 },
  };

  for (auto& tc : test_cases)
  {
    SETUP_PARSER_TEST_CASE(tc.input);
    DEFER({ program_free((Program*) &p); });
    TyEnv env;
    tyenv_init(&env, 16);
    TyObj val = eval(&env, &p);
    ASSERT_EQ(val.kind, TYOBJ_INT)
      << "Expected object to be " << tyobj_kind_to_string(TYOBJ_INT)
      << " (integer), got " << tyobj_kind_to_string(val.kind);
    ASSERT_EQ(val.o.integer.value, tc.exp);
  }
}


///
/// Statement Eval Tests
/// NOTE(HS): not expression_statements, those covered above
///
TEST(EvalTestSuite, Test_Eval_Var_Statement)
{
  struct Eval_Var_Stmt_Tc {
    const char *input;
    const char *ident;
    TyObject_Kind exp_kind;
  };

  auto test_cases = std::vector<Eval_Var_Stmt_Tc>{
    { "var x = 10;", "x", TYOBJ_INT },
    { "var msg = \"Hello, World!\";", "msg", TYOBJ_STRING },
  };

  for (auto& tc : test_cases)
  {
    SETUP_PARSER_TEST_CASE(tc.input);
    DEFER({ program_free((Program*) &p); });

    TyEnv global;
    tyenv_init(&global, 16);
    DEFER({ tyenv_free((TyEnv*) &global); });

    TyObj val = eval(&global, &p);

    ASSERT_EQ(val.kind, TYOBJ_NONE)
      << "Expected result of variable binding to be NONE, got "
      << tyobj_kind_to_string(val.kind);

    // NOTE(HS): assert variable is in environment
    TyObj *obj_inserted = tyenv_get(&global, tc.ident);
    ASSERT_NE(obj_inserted, nullptr);

    // NOTE(HS): check correct value type stored
    ASSERT_EQ(obj_inserted->kind, tc.exp_kind)
      << "Expected variable to store value type " << tyobj_kind_to_string(tc.exp_kind)
      << ", got " << tyobj_kind_to_string(obj_inserted->kind);
  }
}
