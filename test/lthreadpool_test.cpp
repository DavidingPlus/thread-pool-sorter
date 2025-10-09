#include <gtest/gtest.h>

#include <string>

#include "lthreadpool.h"


TEST(TestClassTest, Test1)
{
    EXPECT_EQ(LThreadPool::foo(), std::string("hello world"));
}

TEST(TestClassTest, Test2)
{
    std::pair<int, int> p(3, 4);

    EXPECT_EQ(LThreadPool().gee(3, 4), p);
}

// TEST(TestClassTest, Test3)
// {
//     EXPECT_EQ(1, 3);
// }
