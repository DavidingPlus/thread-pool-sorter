/**
 * @file lrandom.h
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 随机数类头文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#ifndef _LRANDOM_H_
#define _LRANDOM_H_


class LRandom
{

public:

    LRandom() = default;

    ~LRandom() = default;

    static int genRandomNumber(int left, int right);
};


#endif
