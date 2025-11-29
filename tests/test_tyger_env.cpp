///
/// Tests covering the hashmap function of TyEnv type
///
#include <gtest/gtest.h>
#include <vector>
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
TEST(TyEnvTestSuite, TestKeyCollisionResolution)
{
  TyEnv__ env;
  tyenv_init__(&env, 1);  // NOTE(HS): guarentees all insertions are at same index
  int obj_val = 16;
  TyObj *obj = tyobj_new(TYOBJ_INT, &obj_val);
  DEFER({ tyenv_free__((TyEnv__*) &env); });

  auto idents = std::vector<const char*>{ "a", "b", "c", "d", "e", "f", "g", "h", "i", "j" };

  for (auto& ident : idents)
  {
    ASSERT_TRUE(tyenv_insert__(&env, ident, obj))
      << "Failed to inser object with key: " << ident;
  }
}

// TODO(HS): get by key

// TODO(HS): collision resolution in get

// TODO(HS): update existing keys
