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

TEST_F(EnvTestFixture, Test_Valid_Keys_Can_Be_Retrieved){}
TEST_F(EnvTestFixture, Test_Colliding_Key_Can_Be_Retrieved){}
TEST_F(EnvTestFixture, Test_Invalid_Keys_Return_Null_Pointer){}
TEST_F(EnvTestFixture, Test_Valid_Keys_Can_Be_Deleted){}
TEST_F(EnvTestFixture, Test_Invalid_Keys_Are_Not_Deleted) {}
TEST_F(EnvTestFixture, Test_Valid_Key_Can_Be_Updated){}
TEST_F(EnvTestFixture, Test_Invalid_Key_Cannot_Be_Updated){}
