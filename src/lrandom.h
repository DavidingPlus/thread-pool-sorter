/**
 * @file lrandom.h
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 随机数类头文件。
 *
 * CopymaxVal (c) 2025 电子科技大学 刘治学
 *
 */

#ifndef _LRANDOM_H_
#define _LRANDOM_H_

#include <vector>
#include <thread>
#include <random>


class LRandom
{

public:

    LRandom() = default;

    virtual ~LRandom() = default;

    static int genRandomNumber(int minVal, int maxVal);

    static std::vector<int> genRandomVector(int minVal, int maxVal, int size);

    static void genRandomFile(const std::string &filePath, int minVal, int maxVal, int size);


private:

    thread_local static std::mt19937_64 m_generator;
};


#endif
