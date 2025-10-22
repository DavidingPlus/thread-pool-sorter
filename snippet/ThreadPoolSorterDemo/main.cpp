/**
 * @file main.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 线程池排序主程序入口文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <queue>
#include <future>
#include <chrono>
#include <cmath>
#include <cstdio>

#include "lthreadpool.h"
#include "lrandom.h"


constexpr size_t CHUNK_SIZE = 16ULL * 1024 * 1024; // 16 MB
constexpr size_t K = 8;                            // k 路归并，每轮并行合并多少个文件


// ---------------------------
// 写排序后的块
// ---------------------------
std::string writeSortedChunk(const std::string &file, size_t idx, const std::vector<int> &data)
{
    std::string outFile = file + ".part" + std::to_string(idx) + ".sorted";
    std::ofstream ofs(outFile, std::ios::binary);
    ofs.write(reinterpret_cast<const char *>(data.data()), data.size() * sizeof(int));
    return outFile;
}

// ---------------------------
// k 路归并
// ---------------------------
std::string mergeKFiles(const std::vector<std::string> &files, size_t idx)
{
    if (files.empty()) return "";
    if (files.size() == 1) return files[0];

    struct Node
    {
        int val;
        size_t fileIdx;
        bool operator>(const Node &other) const { return val > other.val; }
    };

    std::vector<std::ifstream> streams(files.size());
    for (size_t i = 0; i < files.size(); ++i)
        streams[i].open(files[i], std::ios::binary);

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

    // 初始化堆
    for (size_t i = 0; i < streams.size(); ++i)
    {
        int v;
        if (streams[i].read(reinterpret_cast<char *>(&v), sizeof(v)))
            pq.push({v, i});
    }

    std::string outFile = "tmp_merge_" + std::to_string(idx) + ".bin";
    std::ofstream ofs(outFile, std::ios::binary);

    while (!pq.empty())
    {
        Node node = pq.top();
        pq.pop();
        ofs.write(reinterpret_cast<char *>(&node.val), sizeof(node.val));

        int v;
        if (streams[node.fileIdx].read(reinterpret_cast<char *>(&v), sizeof(v)))
            pq.push({v, node.fileIdx});
    }

    // 关闭并删除源文件
    for (auto &s : streams) s.close();
    for (const auto &f : files) std::remove(f.c_str());

    return outFile;
}

// ---------------------------
// 外部排序
// ---------------------------
void externalSort(const std::string &file, LThreadPool &pool)
{
    std::ifstream ifs(file, std::ios::binary);
    if (!ifs) return;

    std::vector<std::future<std::string>> futures;
    size_t idx = 0;
    std::vector<int> originalData;

    // 输出原始数据前100个
    int val;
    while (ifs.read(reinterpret_cast<char *>(&val), sizeof(val)) && originalData.size() < 100)
        originalData.push_back(val);
    std::cout << "Original data (first 100): ";
    for (auto x : originalData) std::cout << x << " ";
    std::cout << std::endl;

    ifs.clear();
    ifs.seekg(0);
    auto before = std::chrono::high_resolution_clock::now();

    // 分块排序并行
    while (!ifs.eof())
    {
        std::vector<int> buffer;
        buffer.reserve(CHUNK_SIZE / sizeof(int));
        while (buffer.size() < buffer.capacity() && ifs.read(reinterpret_cast<char *>(&val), sizeof(val)))
            buffer.push_back(val);
        if (buffer.empty()) break;

        futures.push_back(pool.enqueue([buffer = std::move(buffer), file, idx]() mutable
                                       {
            std::sort(buffer.begin(), buffer.end());
            return writeSortedChunk(file, idx, buffer); }));
        ++idx;
    }

    std::vector<std::string> sortedFiles;
    for (auto &f : futures) sortedFiles.push_back(f.get());

    // 多轮 k 路归并，每轮并行处理多组 K 个文件
    size_t mergeRound = 0;
    while (sortedFiles.size() > 1)
    {
        std::vector<std::future<std::string>> mergeFutures;
        std::vector<std::string> nextRoundFiles;

        for (size_t i = 0; i < sortedFiles.size(); i += K)
        {
            std::vector<std::string> group;
            for (size_t j = i; j < i + K && j < sortedFiles.size(); ++j)
                group.push_back(sortedFiles[j]);

            if (group.size() == 1)
            {
                nextRoundFiles.push_back(group[0]);
            }
            else
            {
                mergeFutures.push_back(pool.enqueue([group, mergeRound, i]()
                                                    { return mergeKFiles(group, mergeRound * 1000 + i); }));
            }
        }

        for (auto &mf : mergeFutures)
            nextRoundFiles.push_back(mf.get());

        sortedFiles = std::move(nextRoundFiles);
        ++mergeRound;
    }

    auto now = std::chrono::high_resolution_clock::now();
    std::cout << "Sort completed in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(now - before).count()
              << " ms.\n";

    // 重命名最终文件
    std::string finalFile = file + ".sorted.bin";
    std::rename(sortedFiles[0].c_str(), finalFile.c_str());

    // 输出前 100 个排序元素
    std::ifstream sortedFile(finalFile, std::ios::binary);
    std::vector<int> sortedData;
    while (sortedFile.read(reinterpret_cast<char *>(&val), sizeof(val)) && sortedData.size() < 100)
        sortedData.push_back(val);
    std::cout << "Sorted data (first 100): ";
    for (auto x : sortedData) std::cout << x << " ";
    std::cout << std::endl;
}


int main()
{
    const std::string testFile = "test.bin";

    // 生成随机文件
    auto before = std::chrono::high_resolution_clock::now();
    LRandom::genRandomFile(testFile, 0, 1000000, 100000000);
    auto now = std::chrono::high_resolution_clock::now();
    std::cout << "Random file generated in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(now - before).count()
              << " ms.\n";

    // 使用线程池
    LThreadPool pool(12);

    // 外部排序
    externalSort(testFile, pool);


    return 0;
}
