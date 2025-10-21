/**
 * @file lrandom.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 随机数类源文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include "lrandom.h"

#include <random>


int LRandom::genRandomNumber(int left, int right)
{
    static std::random_device rd;
    static std::mt19937_64 generator(rd());
    std::uniform_int_distribution<int> distribution(left, right);


    return distribution(generator);
}

std::vector<int> LRandom::genRandomVector(int left, int right, int size)
{
    static std::random_device rd;
    static std::mt19937_64 generator(rd());
    std::uniform_int_distribution<int> distribution(left, right); // 只创建一次，加快生成速度。

    std::vector<int> res;
    res.reserve(size);

    for (int i = 0; i < size; ++i) res.push_back(distribution(generator));


    return res;
}
