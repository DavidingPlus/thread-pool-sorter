/**
 * @file lrandom.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 随机数类源文件。
 *
 * CopymaxVal (c) 2025 电子科技大学 刘治学
 *
 */

#include "lrandom.h"

#include <fstream>


thread_local std::mt19937_64 LRandom::m_generator(std::random_device{}());


int LRandom::genRandomNumber(int minVal, int maxVal)
{
    std::uniform_int_distribution<int> distribution(minVal, maxVal);


    return distribution(m_generator);
}

std::vector<int> LRandom::genRandomVector(int minVal, int maxVal, int size)
{
    std::uniform_int_distribution<int> distribution(minVal, maxVal);

    std::vector<int> res;
    res.reserve(size);

    for (int i = 0; i < size; ++i) res.push_back(distribution(m_generator));


    return res;
}

void LRandom::genRandomFile(const std::string &filePath, int minVal, int maxVal, int size)
{
    std::ofstream file(filePath, std::ios::binary);
    if (!file) throw std::runtime_error("Failed to open file " + filePath + " to write.");

    std::uniform_int_distribution<int> distribution(minVal, maxVal);

    // 设置 64KB 的写入缓冲区。
    constexpr size_t bufferSize = 1 << 16;
    std::vector<int> buffer;
    buffer.reserve(bufferSize);

    // 文件可能很大，不能直接全部读到 buffer 中一次性写入文件。这里的思路是以 bufferSize 为单位进行切分，分批次写入文件。
    for (int i = 0; i < size; ++i)
    {
        buffer.push_back(distribution(m_generator));
        if (bufferSize == buffer.size())
        {
            // 写入文件的方式是通过 buffer 的内部数组指针，按照字节流的方式写入数组的数据，这样能提高效率，避免逐次写入单个值。
            // 在读取的时候自己注意一下读取规范就行了。
            file.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(int));
            buffer.clear();
        }
    }

    // 处理剩余的部分。
    if (!buffer.empty()) file.write(reinterpret_cast<char *>(buffer.data()), buffer.size() * sizeof(int));
}
