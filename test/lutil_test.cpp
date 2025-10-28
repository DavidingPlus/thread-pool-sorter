#include <gtest/gtest.h>

#include "lglobalmacros.h"
#include "lutil.h"


TEST(LUtilTest, ExecutablePathTest)
{
    std::cout << "executableFullPath: " << LUtil::executableFullPath() << std::endl;
    std::cout << "executableDirectory: " << LUtil::executableDirectory() << std::endl;

#ifdef L_OS_WIN32

    EXPECT_EQ(LUtil::executableDirectory() + "gtest-thread-pool-sorter.exe", LUtil::executableFullPath());

#endif

#ifdef L_OS_LINUX

    EXPECT_EQ(LUtil::executableDirectory() + "gtest-thread-pool-sorter", LUtil::executableFullPath());

#endif
}
