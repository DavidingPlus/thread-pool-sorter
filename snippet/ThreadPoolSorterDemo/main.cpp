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
#include <algorithm>
#include <future>
#include <chrono>
#include <fstream>

#include "lthreadpool.h"
#include "lrandom.h"


constexpr int size = 5000000;
constexpr int left = 0;
constexpr int right = 1000000;
constexpr int poolSize = 12;


/**
 * @brief 合并函数，将两个有序子区间合并为一个有序区间。
 *
 * 该函数用于归并排序，将数组 arr 的 [left, mid] 和 [mid+1, right] 两个已排序的子区间合并为一个连续的有序区间。合并过程使用临时数组 tmp 存放结果，最后再移动回原数组。
 *
 * @param arr   待排序数组的引用。
 * @param left  左边子区间的起始索引（包含）。
 * @param mid   左边子区间的结束索引（包含），右边子区间从 mid+1 开始。
 * @param right 右边子区间的结束索引（包含）。
 */
static void merge(std::vector<int> &arr, int left, int mid, int right)
{
    std::vector<int> tmp;
    tmp.reserve(right - left + 1);

    int i = left, j = mid + 1;
    while (i <= mid && j <= right)
    {
        if (arr[i] <= arr[j])
            tmp.push_back(arr[i++]);
        else
            tmp.push_back(arr[j++]);
    }
    while (i <= mid) tmp.push_back(arr[i++]);
    while (j <= right) tmp.push_back(arr[j++]);

    std::move(tmp.begin(), tmp.end(), arr.begin() + left);
}

/**
 * @brief 使用线程池的并行归并排序。
 *
 * 本函数基于“分而治之”的思想，在排序大规模数据时，通过线程池实现
 * 左右子区间的并行排序，以充分利用多核 CPU 的性能。对于小区间或递归
 * 深度较深的情况，自动退化为串行排序，以避免线程调度开销过大。
 *
 * 并行策略：
 * - 对较大区间 [left, right]，将左半区间提交到线程池异步执行；
 * - 右半区间在当前线程同步执行；
 * - 左右两半排序完成后，主线程执行合并操作；
 * - 小区间（小于等于 64）直接使用 std::sort 以减少线程开销；
 * - 限制最大递归深度 depth ≤ 3，以控制任务数量。
 *
 * @param arr   待排序数组引用。
 * @param left  当前排序区间左边界。
 * @param right 当前排序区间右边界。
 * @param pool  线程池实例引用，用于任务并行。
 * @param depth 当前递归深度，用于控制并行层数。
 */
static void parallelMergeSort(std::vector<int> &arr, int left, int right, LThreadPool &pool, int depth = 0)
{
    // 递归终止条件：单个元素或空区间无需排序。
    if (left >= right)
        return;

    int len = right - left + 1;

    // 若区间过小或递归层数过深，直接在当前线程中使用 std::sort 排序。
    if (len <= 64 || depth >= 3)
    {
        std::sort(arr.begin() + left, arr.begin() + right + 1);
        return;
    }

    // 计算中点，将区间 [left, right] 一分为二。
    int mid = (left + right) / 2;

    // 左半区间交由线程池异步执行。
    auto leftFuture = pool.enqueue([&arr, left, mid, &pool, depth]()
                                   { 
        // 异步任务：递归排序左半区间。
        parallelMergeSort(arr, left, mid, pool, depth + 1); });

    // 当前线程继续排序右半区间（同步执行）。
    parallelMergeSort(arr, mid + 1, right, pool, depth + 1);

    // 等待左半区间排序任务完成。
    leftFuture.get();

    // 合并左右两个有序区间。
    merge(arr, left, mid, right);
}


int main()
{
    // 1. 生成 num 个测试数据。
    auto before = std::chrono::high_resolution_clock::now();
    std::vector<int> vec = LRandom::genRandomVector(left, right, size);
    auto now = std::chrono::high_resolution_clock::now();

    std::cout << "Random vec generated in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(now - before).count()
              << " ms.\n";

    std::ofstream beforeFile("before.txt");
    for (auto &e : vec) beforeFile << e << ' ';
    beforeFile.close();

    // 2. 创建线程池。
    LThreadPool pool(poolSize);

    // 3. 启动并行归并排序。
    before = std::chrono::high_resolution_clock::now();
    parallelMergeSort(vec, 0, size - 1, pool);
    now = std::chrono::high_resolution_clock::now();

    std::cout << "Parallel merge sort completed in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(now - before).count()
              << " ms.\n";

    // 4. 输出排序结果到文件。
    std::ofstream afterFile("after.txt");
    for (auto &e : vec) afterFile << e << ' ';
    afterFile.close();


    return 0;
}
