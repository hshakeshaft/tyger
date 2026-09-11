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

// TODO: test that insertion of existing key updates value
//   - [ ] no collisions
//   - [ ] when collisions occur


// TODO: test retrieval
//  - [ ] test retrieval when no values in table (===> return NULL)
//  - [ ] test retrieval of lone key which exists
//  - [ ] test retrieval of value when multiple in table (no collisions)
//  - [ ] test retrieval when collision resolution

// TODO: test deletion of key
//  - [ ] deletion of key which doesn't exist (nothing happens - return false)
//  - [ ] test deletion of key which exists (and is lone)
//  - [ ] test deleteion of key which exists when there's several
//    - assert other element not deleted
//  - [ ] test deletion of key which collides
