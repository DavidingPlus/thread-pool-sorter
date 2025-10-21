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
