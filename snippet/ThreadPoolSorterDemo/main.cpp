/**
 * @file main.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 线程池排序主程序入口文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include <iostream>
#include <vector>

#include "lthreadpool.h"
#include "lrandom.h"


constexpr int num = 100;


int main()
{
    // 1. 生成 num 个测试数据。
    std::vector<int> data(num);
    for (int i = 0; i < num; ++i)
    {
        data[i] = LRandom::genRandomNumber(0, 10000);
        std::cout << data[i] << ' ' << std::flush;
    }
    std::cout << std::endl;


    // TODO


    return 0;
}
