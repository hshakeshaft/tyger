///
/// Test Hash Table - this suite is the appropriate place for testing full behaviour
/// of the hash table, utilised for storing identifiers in the VM.
///
/// The exact implementation of said hash table should not be tested in the VM,
/// and as such the VM wraps all hash table functions; however the hash table is
/// integrally linked with the concept of a `TyObject` and thus cannot be divoreced.
///
#include <gtest/gtest.h>
#include "helpers/helpers.hpp"
#include "helpers/tyger.hpp"


/// Helper type which manages the lifetime of a TyObject within the hash tablke
struct HTTestValue
{
    HTTestValue(std::string&& t_ident, int t_value)
        : ident{t_ident}, value{t_value}
    {
        this->object = tyobject_create(OBJ_INTEGER, &t_value);
    }

    ~HTTestValue()
    {
        tyobject_destroy(this->object);
    }

    std::string ident;
    TyObject *object;
    int value;
};



TEST(HashTableTestSuite, user_can_insert_into_hash_table)
{
    HT ht;
    ht_init(&ht, 1);

    auto val = HTTestValue("foo", 100);
    ht_insert(&ht, val.ident.c_str(), val.object);

    HT_Slot slot = ht[0];
    ASSERT_EQ(std::string(slot.key), val.ident);
    ASSERT_EQ(slot.value->as.integer, val.value);

    ht_deinit(&ht);
}

TEST(HashTableTestSuite, use_can_insert_multiple_non_colliding_values)
{
    /* NOTE(HS): precomputed slot indexes using the following python program
    ```
    def djb2(key: str) -> int:
      hash = 5381
      for c in list(key):
        hash = ((hash << 5) + hash) + ord(c)
      return hash
    print(djb2("foo") % 8)  # 1
    print(djb2("bar") % 8)  # 2
    ```
    */
    HT ht;
    ht_init(&ht, 8);

    auto val1 = HTTestValue("foo", 100);
    auto val2 = HTTestValue("bar", 50);

    ht_insert(&ht, val1.ident.c_str(), val1.object);
    ht_insert(&ht, val2.ident.c_str(), val2.object);

    HT_Slot slot1 = ht[1];
    HT_Slot slot2 = ht[2];

    ASSERT_EQ(std::string(slot1.key), val1.ident);
    ASSERT_EQ(std::string(slot2.key), val2.ident);

    ASSERT_EQ(slot1.value->as.integer, val1.value);
    ASSERT_EQ(slot2.value->as.integer, val2.value);

    ht_deinit(&ht);
}

TEST(HashTableTestSuite, collision_resolution_on_insert)
{
    HT ht;
    ht_init(&ht, 1);

    // NOTE(HS) this is a stress test, unlikely there will 4 levels of slot indirection
    // but good to check the logic is recursive
    auto val1 = HTTestValue("foo",  100);
    auto val2 = HTTestValue("bar",  50);
    auto val3 = HTTestValue("baz",  25);
    auto val4 = HTTestValue("spam", 12);
    auto val5 = HTTestValue("eggs", 6);

    constexpr int test_case_count = 5;
    HTTestValue *test_cases[test_case_count] = { &val1, &val2, &val3, &val4, &val5 };

    for (auto i = 0; i < test_case_count; ++i)
    {
        HTTestValue * tc = test_cases[i];
        ht_insert(&ht, tc->ident.c_str(), tc->object);
        HT_Slot *slot = &ht[0];
        for (auto idx = 0; idx < i; ++idx) { slot = slot->next; }
        ASSERT_EQ(std::string(slot->key),  tc->ident);
        ASSERT_EQ(slot->value->as.integer, tc->value);
    }

    ht_deinit(&ht);
}

TEST(HashTableTestSuite, reinsertion_of_existing_key_should_update_value)
{
    HT ht;
    ht_init(&ht, 1);

    auto val1 = HTTestValue("foo", 10);
    auto val2 = HTTestValue("foo", 20);

    ht_insert(&ht, val1.ident.c_str(), val1.object);
    ht_insert(&ht, val2.ident.c_str(), val2.object);

    HT_Slot *slot = &ht[0];
    ASSERT_EQ(slot->key, val1.ident);

    ASSERT_NE(slot->value, val1.object);
    ASSERT_EQ(slot->value, val2.object);
    ASSERT_NE(slot->value->as.integer, val1.object->as.integer);
    ASSERT_EQ(slot->value->as.integer, val2.object->as.integer);
}


TEST(HashTableTestSuite, test_retrieval_of_invalid_key_returns_empty_slot)
{
    HT ht;
    ht_init(&ht, 8);
    HT_Slot *slot = ht_get(&ht, "foo");
    ASSERT_EQ(slot->key, nullptr);
    ASSERT_EQ(slot->value, nullptr);
    ASSERT_EQ(slot->next, nullptr);
    ht_deinit(&ht);
}

TEST(HashTableTestSuite, test_retrieval_of_valid_key_returns_slot_to_value)
{
    HT ht;
    ht_init(&ht, 8);

    auto val = HTTestValue("foo", 10);
    ht_insert(&ht, val.ident.c_str(), val.object);

    HT_Slot *slot = ht_get(&ht, val.ident.c_str());
    ASSERT_NE(slot->key, nullptr);
    ASSERT_NE(slot->value, nullptr);
    ASSERT_EQ(std::string(slot->key), val.ident);
    ASSERT_EQ(slot->value->as.integer, val.value);

    ht_deinit(&ht);
}

TEST(HashTableTestSuite, test_retrieval_of_non_colliding_keys_returns_different_slots)
{
    HT ht;
    ht_init(&ht, 8);

    auto val1 = HTTestValue("foo", 10);
    auto val2 = HTTestValue("bar", 50);

    ht_insert(&ht, val1.ident.c_str(), val1.object);
    ht_insert(&ht, val2.ident.c_str(), val2.object);

    HT_Slot *slot1 = ht_get(&ht, val1.ident.c_str());
    HT_Slot *slot2 = ht_get(&ht, val2.ident.c_str());

    ASSERT_NE(slot1, slot2);
    ASSERT_NE(std::string(slot1->key), std::string(slot2->key));
    ASSERT_NE(slot1->value, slot2->value);

    ASSERT_EQ(slot1->key, val1.ident);
    ASSERT_EQ(slot2->key, val2.ident);

    ASSERT_EQ(slot1->value->as.integer, val1.object->as.integer);
    ASSERT_EQ(slot2->value->as.integer, val2.object->as.integer);

    ht_deinit(&ht);
}

TEST(HashTableTestSuite, test_retrieval_of_colliding_keys_resolves_collision)
{
    HT ht;
    ht_init(&ht, 1);

    // NOTE(HS) this is a stress test, unlikely there will 4 levels of slot indirection
    // but good to check the logic is recursive
    auto val1 = HTTestValue("foo",  100);
    auto val2 = HTTestValue("bar",  50);
    auto val3 = HTTestValue("baz",  25);
    auto val4 = HTTestValue("spam", 12);
    auto val5 = HTTestValue("eggs", 6);

    constexpr int test_case_count = 5;
    HTTestValue *test_cases[test_case_count] = { &val1, &val2, &val3, &val4, &val5 };

    for (auto i = 0; i < test_case_count; ++i)
    {
        HTTestValue *tc = test_cases[i];
        ht_insert(&ht, tc->ident.c_str(), tc->object);
    }

    for (auto i = 0; i < test_case_count; ++i)
    {
        HTTestValue *tc = test_cases[i];
        HT_Slot *slot = ht_get(&ht, tc->ident.c_str());
        ASSERT_EQ(slot->key, tc->ident);
        ASSERT_EQ(slot->value->as.integer, tc->object->as.integer);
    }

    ht_deinit(&ht);
}


TEST(HashTableTestSuite, test_deletion_of_non_existing_key_fails)
{
    HT ht;
    ht_init(&ht, 8);
    ASSERT_FALSE(ht_delete(&ht, "foo"));
    ASSERT_FALSE(ht_delete(&ht, "bar"));
    ASSERT_FALSE(ht_delete(&ht, "baz"));
    ht_deinit(&ht);
}

TEST(HashTableTestSuite, test_deleteion_of_existing_key_deletes)
{
    HT ht;
    ht_init(&ht, 8);

    auto val = HTTestValue("foo", 10);
    ht_insert(&ht, val.ident.c_str(), val.object);

    ASSERT_TRUE(ht_delete(&ht, val.ident.c_str()));

    HT_Slot *slot = ht_get(&ht, val.ident.c_str());
    ASSERT_EQ(slot->key, nullptr);
    ASSERT_EQ(slot->value, nullptr);
}

TEST(HashTableTestSuite, test_deleting_one_key_does_not_delete_other_keys)
{
    HT ht;
    ht_init(&ht, 8);

    HT_Slot *slot;
    auto val1 = HTTestValue("foo", 10);
    auto val2 = HTTestValue("bar", 25);
    ht_insert(&ht, val1.ident.c_str(), val1.object);
    ht_insert(&ht, val2.ident.c_str(), val2.object);

    ASSERT_TRUE(ht_delete(&ht, val1.ident.c_str()));

    // test val1 deleted
    slot = ht_get(&ht, val1.ident.c_str());
    ASSERT_EQ(slot->key, nullptr);
    ASSERT_EQ(slot->value, nullptr);

    // test val2 still present
    slot = ht_get(&ht, val2.ident.c_str());
    ASSERT_EQ(slot->key, val2.ident);
    ASSERT_EQ(slot->value->as.integer, val2.object->as.integer);
}

TEST(HashTableTestSuite, test_that_deletion_of_keys_in_chain_resolves)
{
    HT ht;
    ht_init(&ht, 1);

    auto val1 = HTTestValue("foo", 10);
    auto val2 = HTTestValue("bar", 20);
    auto val3 = HTTestValue("baz", 30);

    ht_insert(&ht, val1.ident.c_str(), val1.object);
    ht_insert(&ht, val2.ident.c_str(), val2.object);
    ht_insert(&ht, val3.ident.c_str(), val3.object);

    ASSERT_TRUE(ht_delete(&ht, val2.ident.c_str()));
    
    // Test that `slot[val3]` becomes `slot[val1]->next`
    HT_Slot *slot1 = ht_get(&ht, val1.ident.c_str());
    HT_Slot *slot3 = ht_get(&ht, val3.ident.c_str());
    ASSERT_NE(slot1, nullptr);
    ASSERT_NE(slot1->next, nullptr);
    ASSERT_EQ(slot1->next, slot3);

    // test that deleting `slot[val3]` now sets `slot[val1]->next` to NULL
    ASSERT_TRUE(ht_delete(&ht, val3.ident.c_str()));
    slot1 = ht_get(&ht, val1.ident.c_str());
    ASSERT_EQ(slot1->next, nullptr);
}

TEST(HashTableTestSuite, test_deletion_of_initial_slot_moves_back_colission_chain)
{
    HT ht;
    ht_init(&ht, 1);

    HT_Slot *slot;
    auto val1 = HTTestValue("foo", 10);
    auto val2 = HTTestValue("bar", 20);

    ht_insert(&ht, val1.ident.c_str(), val1.object);
    ht_insert(&ht, val2.ident.c_str(), val2.object);

    ASSERT_TRUE(ht_delete(&ht, val1.ident.c_str()));

    // test that deleting the first key moves next back into this slot
    slot = ht_get(&ht, val2.ident.c_str());
    ASSERT_EQ(slot->key, val2.ident);
    ASSERT_EQ(slot->value->as.integer, val2.object->as.integer);
    ASSERT_EQ(slot->next, nullptr);

    // test that deleting the first key in a slot when no others available "resets"
    // all slot values to NULL
    ASSERT_TRUE(ht_delete(&ht, val2.ident.c_str()));
    slot = ht_get(&ht, val2.ident.c_str());
    ASSERT_EQ(slot->key,   nullptr);
    ASSERT_EQ(slot->value, nullptr);
    ASSERT_EQ(slot->next,  nullptr);

    ht_deinit(&ht);
}
