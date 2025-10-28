/**
 * @file main.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 线程池排序主程序入口文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include <iostream>
#include <chrono>

#include "lthreadpool.h"
#include "lrandom.h"
#include "lsorter.h"
#include "lutil.h"


int main()
{
    // 生成测试文件。
    const std::string testFilePath = LUtil::executableDirectory() + "test.bin";

    auto before = std::chrono::high_resolution_clock::now();
    LRandom::genRandomFile(testFilePath, 0, 1000000, 10000000);
    auto now = std::chrono::high_resolution_clock::now();
    std::cout << "Random file generated in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(now - before).count()
              << " ms.\n";

    // 创建线程池。
    LThreadPool pool(12);

    // 开始线程池排序。
    LSorter sorter(&pool);

    before = std::chrono::high_resolution_clock::now();
    sorter.run(testFilePath);
    now = std::chrono::high_resolution_clock::now();
    std::cout << "Sort completed in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(now - before).count()
              << " ms.\n";


    return 0;
}
