///
/// Tests covering the hashmap function of Env type
///   NOTE(HS): these are the tests which coincide with the refactor
///
#include <gtest/gtest.h>
#include <vector>
#include <stdint.h>
#include "parser_test_helper.hpp"
#include "tyger_test.hpp"

class EnvTestFixture : public ::testing::Test
{
public:
    EnvTestFixture() : ::testing::Test()
    {
        this->env = new Env;
        env_init(this->env, 16);
    }

    ~EnvTestFixture()
    {
        env_deinit(env);
        delete this->env;
    }

protected:
    Env *env;
};


TEST_F(EnvTestFixture, Test_New_keys_Can_Be_Inserted)
{
    int val = 10;
    auto obj = tyobj_new(TYOBJ_INT, &val);
    auto key = std::string{"obj"};
    auto res = env_insert(this->env, key.c_str(), obj);
    ASSERT_TRUE(res) << "Failed to insert object into hashmap with key " << key;

    auto hm_get = env_get(this->env, key.c_str());
    ASSERT_NE(hm_get, nullptr);
    ASSERT_EQ(hm_get->kind, obj->kind);
    ASSERT_EQ(hm_get->o.integer.value, obj->o.integer.value);
}

TEST_F(EnvTestFixture, Test_Collisions_On_Insert_Are_handled)
{
    // NOTE(HS): hack required for this case
    env_deinit(this->env);
    env_init(this->env, 1);

    int val1 = 10;
    auto key1 = std::string{"foo"};
    auto obj1 = tyobj_new(TYOBJ_INT, &val1);

    int val2 = 15;
    auto obj2 = tyobj_new(TYOBJ_INT, &val2);
    auto key2 = std::string{"bar"};

    ASSERT_TRUE(env_insert(this->env, key1.c_str(), obj1));
    ASSERT_TRUE(env_insert(this->env, key2.c_str(), obj2));

    { // NOTE(HS): test that insertion of colliding key doesn't evict original
        TyObj *obj1_get = env_get(this->env, key1.c_str());
        ASSERT_NE(obj1_get, nullptr);
        ASSERT_EQ(obj1_get->kind, obj1->kind);
        ASSERT_EQ(obj1_get->o.integer.value, obj1->o.integer.value);

        TyObj *obj2_get = env_get(this->env, key2.c_str());
        ASSERT_NE(obj2_get, nullptr);
        ASSERT_EQ(obj2_get->kind, obj2->kind);
        ASSERT_EQ(obj2_get->o.integer.value, obj2->o.integer.value);
    }
}

TEST_F(EnvTestFixture, Test_That_The_Same_Key_Cannot_Be_Reinserted)
{
  // NOTE(HS): hack required for this case
  env_deinit(this->env);
  env_init(this->env, 1);

  int val1 = 10;
  auto key1 = std::string{"foo"};
  auto obj1 = tyobj_new(TYOBJ_INT, &val1);

  int val2 = 15;
  auto obj2 = tyobj_new(TYOBJ_INT, &val2);
  auto key2 = std::string{"foo"};

  ASSERT_TRUE(env_insert(this->env, key1.c_str(), obj1));
  ASSERT_FALSE(env_insert(this->env, key2.c_str(), obj2));

  // NOTE(HS): check 1st entry isn't munged by failed reinsertion
  auto obj1_get = env_get(this->env, key1.c_str());
  ASSERT_NE(obj1_get, nullptr);
  ASSERT_EQ(obj1_get->kind, obj1->kind);
  ASSERT_EQ(obj1_get->o.integer.value, obj1->o.integer.value);
}

TEST_F(EnvTestFixture, Test_Valid_Keys_Can_Be_Retrieved)
{
    int val = 10;
    auto obj = tyobj_new(TYOBJ_INT, &val);
    auto key = std::string{"foo"};

    env_insert(this->env, key.c_str(), obj);

    auto res = env_get(this->env, key.c_str());
    ASSERT_NE(res, nullptr);
    ASSERT_EQ(res->kind, obj->kind);
    ASSERT_EQ(res->o.integer.value, res->o.integer.value);
}

TEST_F(EnvTestFixture, Test_Colliding_Key_Can_Be_Retrieved)
{
  env_deinit(this->env);
  env_init(this->env, 1);

  int val1 = 10;
  auto key1 = std::string{"foo"};
  auto obj1 = tyobj_new(TYOBJ_INT, &val1);

  int val2 = 15;
  auto obj2 = tyobj_new(TYOBJ_INT, &val2);
  auto key2 = std::string{"bar"};

  env_insert(this->env, key1.c_str(), obj1);
  env_insert(this->env, key2.c_str(), obj2);

  { // check initial insert can be retrieved
    auto res1 = env_get(this->env, key1.c_str());
    ASSERT_NE(res1, nullptr);
    ASSERT_EQ(res1->kind, obj1->kind);
    ASSERT_EQ(res1->o.integer.value, obj1->o.integer.value);
  }

  { // check colliding insert can be retrieved
    auto res2 = env_get(this->env, key2.c_str());
    ASSERT_NE(res2, nullptr);
    ASSERT_EQ(res2->kind, obj2->kind);
    ASSERT_EQ(res2->o.integer.value, obj2->o.integer.value);
  }
}

TEST_F(EnvTestFixture, Test_Invalid_Keys_Return_Null_Pointer)
{
  { // when no keys inserted ino HM then nothing should return valid object
    auto keys = std::vector<std::string>{
      "foo", "bar", "baz"
    };

    for (const auto& key : keys)
    {
      auto res = env_get(this->env, key.c_str());
      ASSERT_EQ(res, nullptr);
    }
  }

  { // when there are keys, but the queries one does not exist then NULL should be
    // returned
    int val = 10;
    auto obj = tyobj_new(TYOBJ_INT, &val);
    env_insert(this->env, "foo", obj);
    env_insert(this->env, "bar", obj);

    auto res = env_get(this->env, "baz");
    ASSERT_EQ(res, nullptr);
  }
}

TEST_F(EnvTestFixture, Test_Valid_Keys_Can_Be_Deleted){}
TEST_F(EnvTestFixture, Test_Invalid_Keys_Are_Not_Deleted) {}
TEST_F(EnvTestFixture, Test_Valid_Key_Can_Be_Updated){}
TEST_F(EnvTestFixture, Test_Invalid_Key_Cannot_Be_Updated){}
