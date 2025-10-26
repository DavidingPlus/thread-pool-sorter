/**
 * @file lsorter.h
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 线程池排序类头文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#ifndef _LSORTER_H_
#define _LSORTER_H_

#include <string>
#include <vector>

#include "lthreadpool.h"


/**
 * @class LSorter
 * @brief 提供线程池并发的排序功能。
 * @details 当前算法的核心思想：
 * 1. 将大文件分块 chunk 加载到内存，使用线程池对每块进行排序并写入临时文件。
 * 2. 对排好序的临时文件进行 k 路归并，每轮可并行处理多组文件，最终生成排序结果。
 */
class LSorter
{
public:

    /**
     * @brief 构造函数。
     * @param pool 外部线程池指针，用于并行排序和归并任务。
     * @param chunkSize 每块内存大小，默认 16 MB。
     * @param k k 路归并，每轮并行处理的文件数量，默认 8。
     */
    LSorter(LThreadPool *pool, unsigned int chunkSize = 16 * 1024 * 1024, unsigned int k = 8);

    /**
     * @brief 析构函数。
     */
    virtual ~LSorter();

    /**
     * @brief 执行整个排序算法，最终生成 xxx.sorted 文件。
     * @param filePath 待排序文件路径。
     */
    void run(const std::string &filePath);

private:

    /**
     * @brief 将单个块排序后写入临时文件。
     * @param filePath 原始文件名，用于生成临时文件名。
     * @param index 块索引。
     * @param data 排序后的数据。
     * @return 返回生成的临时文件名。
     */
    std::string writeSortedChunk(const std::string &filePath, unsigned int index, const std::vector<int> &data);

    /**
     * @brief k 路归并算法。
     * @param filePaths 待归并文件路径列表。
     * @param index 当前归并轮次索引，用于生成临时文件名。
     * @return 返回归并后的新文件路径。
     */
    std::string mergeKFiles(const std::vector<std::string> &filePaths, unsigned int index);


private:

    /**
     * @brief 外部线程池指针。
     */
    LThreadPool *m_pool = nullptr;

    /**
     * @brief 内存池的块大小。
     */
    unsigned int m_chunkSize = 0;

    /**
     * @brief k 路归并的文件数量。
     */
    unsigned int m_k = 0;
};


#endif
