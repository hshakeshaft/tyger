#include <gtest/gtest.h>
#include "foo.h"

TEST(FooTestSuite, Test_square_works)
{
  int exp = 4;
  int res = squarei(2);
  EXPECT_EQ(res, exp);
}

TEST(FooTestSuite, Test_add_works)
{
  int exp = 6;
  int res = addi(2, 4);
  EXPECT_EQ(exp, res);
}
