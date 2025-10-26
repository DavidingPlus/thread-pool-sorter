/**
 * @filePath lsorter.h
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 线程池排序类头文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#ifndef _LSORTER_H_
#define _LSORTER_H_

#include <string>

#include "lthreadpool.h"


class LSorter
{

public:

    LSorter(LThreadPool *pool, unsigned int chunkSize = 16 * 1024 * 1024, unsigned int k = 8);

    virtual ~LSorter() = default;

    void run(const std::string &filePath);


private:

    std::string writeSortedChunk(const std::string &filePath, unsigned int index, const std::vector<int> &data);

    std::string mergeKfiles(const std::vector<std::string> &filePaths, unsigned int index);


private:

    LThreadPool *m_pool = nullptr;

    unsigned int m_chunkSize = 0;

    unsigned int m_k = 0;
};


#endif
