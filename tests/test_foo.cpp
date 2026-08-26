#include <gtest/gtest.h>

#include "foo.h"

TEST(FooTestSuite, test_square_i32)
{
    ASSERT_EQ(square_i32(2), 4);
    ASSERT_EQ(square_i32(-2), 4);
}

TEST(FooTestSuite, test_pow_i32)
{
    ASSERT_EQ(pow_i32(2, 2), 4);
    ASSERT_EQ(pow_i32(-2, 3), -8);
}
