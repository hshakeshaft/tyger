///
/// Tests covering the hashmap function of TyEnv type
///
#include <gtest/gtest.h>
#include "parser_test_helper.hpp"
#include "tyger_test.hpp"

// TODO(HS): refactor to remove `__` suffix when this works and works well

TEST(TyEnvTestSuite, TestNewKeyInsertion)
{
  TyEnv__ env;
  tyenv_init__(&env, 16);
  int obj_val = 16;
  TyObj *obj = tyobj_new(TYOBJ_INT, &obj_val);
  DEFER({ tyenv_free__((TyEnv__*) &env); });
  ASSERT_TRUE(tyenv_insert__(&env, "x", obj)) << "Failed to insert object with key `x`";
}

TEST(TyEnvTestSuite, TestMultipleKeyInsertion)
{
  TyEnv__ env;
  tyenv_init__(&env, 16);
  int obj_val = 16;
  TyObj *obj = tyobj_new(TYOBJ_INT, &obj_val);
  DEFER({ tyenv_free__((TyEnv__*) &env); });
  ASSERT_TRUE(tyenv_insert__(&env, "x", obj)) << "Failed to insert object with key `x`";
  ASSERT_TRUE(tyenv_insert__(&env, "y", obj)) << "Failed to insert object with key `y`";
}

// TOOD(HS): collision resolution on insert

// TODO(HS): get by key

// TODO(HS):collision resplution on get
