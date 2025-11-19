#include <gtest/gtest.h>
#include <cstdint>
#include <vector>
#include "tyger_test.hpp"
#include "../tests/parser_test_helper.hpp"

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
    TyObj val = eval(&p);
    ASSERT_EQ(val.kind, TYOBJ_INT)
      << "Expected object to be " << tyobj_kind_to_string(TYOBJ_INT)
      << " (integer), got " << tyobj_kind_to_string(val.kind);
    ASSERT_EQ(val.o.integer.value, tc.exp);
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
    TyObj val = eval(&p);
    ASSERT_EQ(val.kind, TYOBJ_INT)
      << "Expected object to be " << tyobj_kind_to_string(TYOBJ_INT)
      << " (integer), got " << tyobj_kind_to_string(val.kind);
    ASSERT_EQ(val.o.integer.value, tc.exp);
  }
}
