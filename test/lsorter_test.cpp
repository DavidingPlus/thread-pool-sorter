#include <gtest/gtest.h>

#include "lsorter.h"


TEST(LSorterTest, Test1)
{
    EXPECT_THROW(
        {
            LSorter(nullptr);
        },
        std::runtime_error);
}
