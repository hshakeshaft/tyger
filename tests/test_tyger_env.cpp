///
/// Tests covering the hashmap function of TyEnv type
///
#include <gtest/gtest.h>
#include <vector>
#include <stdint.h>
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

TEST(TyEnvTestSuite, TestValidKeyRetrieved)
{
  TyEnv__ env;
  tyenv_init__(&env, 16);
  int64_t obj_val = 16;
  TyObj *obj = tyobj_new(TYOBJ_INT, &obj_val);
  DEFER({ tyenv_free__((TyEnv__*) &env); });
  ASSERT_TRUE(tyenv_insert__(&env, "x", obj));

  TyObj *res = tyenv_get__(&env, "x");
  ASSERT_NE(res, nullptr);
  ASSERT_EQ(res->kind, obj->kind)
    << "Expected retrieved object to be of type " << tyobj_kind_to_string(obj->kind)
    << ", got " << tyobj_kind_to_string(res->kind);
  ASSERT_EQ(res->o.integer.value, obj_val);
}

TEST(TyEnvTestSuite, TestKeyGetResolution)
{
  TyEnv__ env;
  tyenv_init__(&env, 1);  // NOTE(HS): guarentees hash collision

  int64_t v1 = 10, v2 = 20;
  TyObj *o1 = tyobj_new(TYOBJ_INT, &v1), *o2 = tyobj_new(TYOBJ_INT, &v2);

  DEFER({ tyenv_free__((TyEnv__*) &env); });

  ASSERT_TRUE(tyenv_insert__(&env, "x", o1));
  ASSERT_TRUE(tyenv_insert__(&env, "y", o2));

  TyObj *res1 = tyenv_get__(&env, "x");
  TyObj *res2 = tyenv_get__(&env, "y");
  ASSERT_NE(res1, nullptr);
  ASSERT_NE(res2, nullptr);
  ASSERT_NE(res1, res2);

  ASSERT_EQ(res1->kind, TYOBJ_INT);
  ASSERT_EQ(res2->kind, TYOBJ_INT);

  ASSERT_EQ(res1->o.integer.value, v1);
  ASSERT_EQ(res2->o.integer.value, v2);
}

TEST(TyEnvTestSuite, TestInvalidKeyLookupReturnsNULL)
{
  TyEnv__ env;
  tyenv_init__(&env, 16);
  DEFER({ tyenv_free__((TyEnv__*) &env); });

  auto idents = std::vector<const char *>{
    "x", "foo", "seven",
    "flocinocipifilification",
    "supercalifragilisticexpialidocious",
  };

  for (auto& ident : idents)
  {
    ASSERT_EQ(tyenv_get__(&env, ident), nullptr)
      << "Expected key \"" << ident << "\" to return NULL (i.e. be invalid)";
  }
}


// NOTE(HS): no language semantics yet to support this but may as well include
TEST(TyEnvTestSuite, TestExistingKeysCanBeUpdated)
{
  TyEnv__ env;
  tyenv_init__(&env, 16);

  int64_t init =16, update1 = 7, update2 = 0;
  TyObj *o1 = tyobj_new(TYOBJ_INT, &init);
  TyObj *o2 = tyobj_new(TYOBJ_INT, &update1);
  TyObj *o3 = tyobj_new(TYOBJ_INT, &update2);

  DEFER({ tyenv_free__((TyEnv__*) &env); });

  tyenv_insert__(&env, "x", o1);

  { // "re-inserting" existing key should work
    ASSERT_TRUE(tyenv_insert__(&env, "x", o2));
    TyObj *res = tyenv_get__(&env, "x");
    ASSERT_NE(res, nullptr);
    ASSERT_EQ(res->kind, o2->kind)
      << "Expected object to be updated to have type " << tyobj_kind_to_string(o2->kind)
      << ", got " << tyobj_kind_to_string(res->kind);
    ASSERT_EQ(res->o.integer.value, o2->o.integer.value);
  }

  { // explicit call to update should work also
    ASSERT_TRUE(tyenv_update__(&env, "x", o3));
    TyObj *res = tyenv_get__(&env, "x");
    ASSERT_NE(res, nullptr);
    ASSERT_EQ(res->kind, o3->kind)
      << "Expected object to be updated to have type " << tyobj_kind_to_string(o3->kind)
      << ", got " << tyobj_kind_to_string(res->kind);
    ASSERT_EQ(res->o.integer.value, o3->o.integer.value);
  }

  const char *sval = "foo";
  TyObj *o4 = tyobj_new(TYOBJ_STRING, &sval);
  { // check types update too
    ASSERT_TRUE(tyenv_update__(&env, "x", o4));
    TyObj *res = tyenv_get__(&env, "x");
    ASSERT_NE(res, nullptr);
    ASSERT_EQ(res->kind, o4->kind)
      << "Expected object to be updated to have type " << tyobj_kind_to_string(o4->kind)
      << ", got " << tyobj_kind_to_string(res->kind);

    auto res_str = std::string{res->o.string.value};
    auto exp_str = std::string{o4->o.string.value};
    ASSERT_EQ(res_str, exp_str);
  }
}


// TODO(HS): handle key deletions
// NOTE(HS): no language semantics yet to support this but may as well include
