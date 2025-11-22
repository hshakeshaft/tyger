#include <gtest/gtest.h>
#include <cstdint>
#include <vector>
#include <string>
#include "tyger_test.hpp"
#include "../tests/parser_test_helper.hpp"

/// Custom fixture for evaluation tests - makes life easier with less macro hell
class TestEval : public ::testing::Test
{
protected:
  Lexer l;
  Parser parser;
  Program prog;
  TyEnv env;

  void setup_test(const char *input)
  {
    lexer_init(&this->l, input);
    parser_init(&this->parser, &this->l);
    this->prog = parser_parse_program(&this->parser);
    tyenv_init(&this->env, 16);
  }

  void teardown_test()
  {
    program_free(&this->prog);
    tyenv_free(&this->env);
  }

  static void assert_object_type_is(TyObj act, TyObject_Kind exp)
  {
    ASSERT_EQ(act.kind, exp) 
      << "Expected Object of type [" << tyobj_kind_to_string(exp)
      << "], got [" << tyobj_kind_to_string(act.kind) << "]";
  }

  static void assert_object_value_eq(TyObj act, int64_t exp)
  {
    assert_object_type_is(act, TYOBJ_INT);
    ASSERT_EQ(act.o.integer.value, exp);
  }

  static void assert_object_value_eq(TyObj act, const char *exp, size_t exp_len)
  {
    assert_object_type_is(act, TYOBJ_STRING);
    ASSERT_EQ(act.o.string.len, exp_len) << "String lengths do not match";
    auto act_s = std::string{act.o.string.value, act.o.string.len};
    auto exp_s = std::string{exp, exp_len};
    ASSERT_EQ(exp_s, act_s);
  }

  // TODO(HS): dump environment if can't find symbol?
  void assert_var_added_to_env(const char *ident)
  {
    TyObj *stored = tyenv_get(&this->env, ident);
    ASSERT_NE(stored, nullptr) << "Variable with ident [" << ident << "] not found in environment";
  }

  void assert_env_var_has_type(const char *ident, TyObject_Kind exp)
  {
    assert_var_added_to_env(ident);
    TyObj *stored = tyenv_get(&this->env, ident);
    assert_object_type_is(*stored, exp);
  }
};

///
/// Expression Eval Tests
///

TEST_F(TestEval, Test_Eval_Int_Expression)
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
    setup_test(tc.input);
    DEFER({ teardown_test(); });
    TyObj act = eval(&this->env, &this->prog);
    assert_object_value_eq(act, tc.exp);
  }
}

TEST_F(TestEval, Test_Eval_String_Expressions)
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
    setup_test(tc.input);
    DEFER({ teardown_test(); });
    TyObj act = eval(&this->env, &this->prog);
    assert_object_value_eq(act, tc.exp, tc.exp_len);
  }
}

TEST_F(TestEval, Test_Eval_Infix_Expression)
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
    setup_test(tc.input);
    DEFER({ teardown_test(); });
    TyObj act = eval(&this->env, &this->prog);
    assert_object_value_eq(act, tc.exp);
  }
}


///
/// Statement Eval Tests
/// NOTE(HS): not expression_statements, those covered above
///
TEST_F(TestEval, Test_Eval_Var_Statement)
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
    setup_test(tc.input);
    DEFER({ teardown_test(); });
    TyObj act = eval(&this->env, &this->prog);
    assert_object_type_is(act, TYOBJ_NONE);
    assert_var_added_to_env(tc.ident);
    assert_env_var_has_type(tc.ident, tc.exp_kind);
  }
}
