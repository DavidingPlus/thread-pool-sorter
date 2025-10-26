#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "lthreadpool.h"


TEST(LThreadPoolTest, Test1)
{
    LThreadPool pool(4);
    std::vector<std::future<int>> res;

    for (int i = 0; i < 8; ++i)
    {
        res.emplace_back(
            pool.enqueue([i] { //
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i * i;
            }));
    }

    for (int i = 0; i < 8; ++i)
    {
        // 注意：std::future::get() 只能调用一次，一旦调用后 future 的状态被消耗，
        // 再次调用会抛出 std::future_error 异常。因此不能重复使用 res[i].get()。
        // 如果需要多次访问结果，可以将结果先保存到变量或使用 std::shared_future。
        int val = res[i].get();
        std::cout << val << std::endl;

        EXPECT_EQ(val, i * i);
    }
}
