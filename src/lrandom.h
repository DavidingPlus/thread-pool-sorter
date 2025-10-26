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


/**
 * @class LRandom
 * @brief 提供静态方法用于生成随机整数数组和文件。
 */
class LRandom
{
public:

    /**
     * @brief 默认构造函数。
     */
    LRandom() = default;

    /**
     * @brief 默认析构函数。
     */
    virtual ~LRandom() = default;

    /**
     * @brief 生成指定范围内的单个随机整数。
     * @param minVal 随机数下界（包含）。
     * @param maxVal 随机数上界（包含）。
     * @return 随机整数。
     */
    static int genRandomNumber(int minVal, int maxVal);

    /**
     * @brief 生成一个指定长度的随机整数向量。
     * @param minVal 随机数下界（包含）。
     * @param maxVal 随机数上界（包含）。
     * @param size 向量长度。
     * @return 随机整数向量。
     *
     * @note 内部多次调用 genRandomNumber。
     */
    static std::vector<int> genRandomVector(int minVal, int maxVal, int size);

    /**
     * @brief 生成指定大小的随机整数文件。
     * @param filePath 输出文件路径。
     * @param minVal 随机数下界（包含）。
     * @param maxVal 随机数上界（包含）。
     * @param size 文件中随机整数的个数。
     */
    static void genRandomFile(const std::string &filePath, int minVal, int maxVal, int size);


private:

    /**
     * @brief 线程局部静态随机数引擎，避免多线程竞争。
     * @note 每个线程独立维护自身的随机引擎实例，提高并发环境下的性能与随机质量。
     */
    thread_local static std::mt19937_64 m_generator;
};


#endif
