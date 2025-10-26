/**
 * @file lsorter.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 线程池排序类源文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include "lsorter.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <future>


LSorter::LSorter(LThreadPool *pool, unsigned int chunkSize, unsigned int k) : m_pool(pool), m_chunkSize(chunkSize), m_k(k)
{
    if (!pool) throw std::runtime_error("Pointer pool is nullptr.");
}

void LSorter::run(const std::string &filePath)
{
    std::ifstream ifs(filePath, std::ios::binary);
    if (!ifs) return;

    std::vector<std::future<std::string>> futures;
    size_t index = 0;
    std::vector<int> originalData;

    // 输出原始数据前 100 个。
    int val;
    while (ifs.read(reinterpret_cast<char *>(&val), sizeof(val)) && originalData.size() < 100) originalData.push_back(val);

    std::cout << "Original data (first 100): ";
    for (auto x : originalData) std::cout << x << " ";
    std::cout << std::endl;

    ifs.clear();
    ifs.seekg(0);

    // 分块排序并行。
    while (!ifs.eof())
    {
        std::vector<int> buffer;
        buffer.reserve(m_chunkSize / sizeof(int));
        while (buffer.size() < buffer.capacity() && ifs.read(reinterpret_cast<char *>(&val), sizeof(val))) buffer.push_back(val);
        if (buffer.empty()) break;

        futures.push_back(m_pool->enqueue([buffer = std::move(buffer), filePath, index, this]() mutable
                                          {std::sort(buffer.begin(), buffer.end());
            return writeSortedChunk(filePath, index, buffer); }));

        ++index;
    }

    std::vector<std::string> sortedFiles;
    for (auto &f : futures) sortedFiles.push_back(f.get());

    // 多轮 k 路归并，每轮并行处理多组 K 个文件。
    size_t mergeRound = 0;
    while (sortedFiles.size() > 1)
    {
        std::vector<std::future<std::string>> mergeFutures;
        std::vector<std::string> nextRoundFiles;

        for (size_t i = 0; i < sortedFiles.size(); i += m_k)
        {
            std::vector<std::string> group;
            for (size_t j = i; j < i + m_k && j < sortedFiles.size(); ++j) group.push_back(sortedFiles[j]);

            if (1 == group.size())
            {
                nextRoundFiles.push_back(group[0]);
            }
            else
            {
                mergeFutures.push_back(m_pool->enqueue([group, mergeRound, i, this]()
                                                       { return mergeKfiles(group, mergeRound * 1000 + i); }));
            }
        }

        for (auto &mf : mergeFutures) nextRoundFiles.push_back(mf.get());

        sortedFiles = std::move(nextRoundFiles);
        ++mergeRound;
    }

    // 重命名最终文件。
    std::string finalFilePath = filePath + ".sorted";
    std::rename(sortedFiles[0].c_str(), finalFilePath.c_str());

    // 输出前 100 个排序元素。
    std::ifstream sortedFile(finalFilePath, std::ios::binary);
    std::vector<int> sortedData;
    while (sortedFile.read(reinterpret_cast<char *>(&val), sizeof(val)) && sortedData.size() < 100) sortedData.push_back(val);

    std::cout << "Sorted data (first 100): ";
    for (auto x : sortedData) std::cout << x << " ";
    std::cout << std::endl;
}

std::string LSorter::writeSortedChunk(const std::string &filePath, unsigned int index, const std::vector<int> &data)
{
    std::string outputFilePath = filePath + ".part" + std::to_string(index) + ".sorted";

    std::ofstream ofs(outputFilePath, std::ios::binary);
    ofs.write(reinterpret_cast<const char *>(data.data()), data.size() * sizeof(int));


    return outputFilePath;
}

std::string LSorter::mergeKfiles(const std::vector<std::string> &filePaths, unsigned int index)
{
    if (filePaths.empty()) return std::string();
    if (1 == filePaths.size()) return filePaths[0];

    struct Node
    {
        int val;
        size_t fileindex;
        bool operator>(const Node &other) const { return val > other.val; }
    };

    std::vector<std::ifstream> streams(filePaths.size());
    for (size_t i = 0; i < filePaths.size(); ++i)
        streams[i].open(filePaths[i], std::ios::binary);

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

    // 初始化堆。
    for (size_t i = 0; i < streams.size(); ++i)
    {
        int v;
        if (streams[i].read(reinterpret_cast<char *>(&v), sizeof(v)))
            pq.push({v, i});
    }

    std::string outputFilePath = "tmp_merge_" + std::to_string(index) + ".bin";
    std::ofstream ofs(outputFilePath, std::ios::binary);

    while (!pq.empty())
    {
        Node node = pq.top();
        pq.pop();
        ofs.write(reinterpret_cast<char *>(&node.val), sizeof(node.val));

        int v;
        if (streams[node.fileindex].read(reinterpret_cast<char *>(&v), sizeof(v)))
            pq.push({v, node.fileindex});
    }

    // 关闭并删除源文件。
    for (auto &s : streams) s.close();
    for (const auto &f : filePaths) std::remove(f.c_str());


    return outputFilePath;
}
