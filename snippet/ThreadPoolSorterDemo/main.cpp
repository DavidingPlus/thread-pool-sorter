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
#include <chrono>
#include <queue>
#include <future>

#include "lthreadpool.h"
#include "lrandom.h"


// 每个块的大小，测试用 1MB
constexpr size_t CHUNK_SIZE = 1ULL * 1024 * 1024;


// ---------------------------
// 读取文件块
// ---------------------------
std::vector<int> readChunk(const std::string &file, size_t offset, size_t count)
{
    std::ifstream ifs(file, std::ios::binary);
    ifs.seekg(offset);
    std::vector<int> buffer(count);
    ifs.read(reinterpret_cast<char *>(buffer.data()), count * sizeof(int));
    buffer.resize(ifs.gcount() / sizeof(int));
    return buffer;
}

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
// 多路归并
// ---------------------------
void mergeFiles(const std::vector<std::string> &files, const std::string &outputFile)
{
    struct Node
    {
        int val;
        size_t idx;
        bool operator>(const Node &other) const { return val > other.val; }
    };

    std::vector<std::ifstream> streams(files.size());
    for (size_t i = 0; i < files.size(); ++i)
        streams[i].open(files[i], std::ios::binary);

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    for (size_t i = 0; i < streams.size(); ++i)
    {
        int v;
        if (streams[i].read(reinterpret_cast<char *>(&v), sizeof(v)))
            pq.push({v, i});
    }

    std::ofstream ofs(outputFile, std::ios::binary);
    while (!pq.empty())
    {
        Node node = pq.top();
        pq.pop();
        ofs.write(reinterpret_cast<char *>(&node.val), sizeof(node.val));

        int v;
        if (streams[node.idx].read(reinterpret_cast<char *>(&v), sizeof(v)))
            pq.push({v, node.idx});
    }
}

// ---------------------------
// 外部排序单文件
// ---------------------------
void externalSort(const std::string &file, LThreadPool &pool)
{
    std::ifstream ifs(file, std::ios::binary);
    if (!ifs) return;

    std::vector<std::future<std::string>> futures;
    size_t idx = 0;
    std::vector<int> originalData;

    // 输出原始数据（前100个）
    int val;
    while (ifs.read(reinterpret_cast<char *>(&val), sizeof(val)) && originalData.size() < 100)
        originalData.push_back(val);
    std::cout << "Original data (first 100): ";
    for (auto x : originalData) std::cout << x << " ";
    std::cout << std::endl;

    ifs.clear();
    ifs.seekg(0);

    // 分块排序
    while (!ifs.eof())
    {
        std::vector<int> buffer;
        buffer.reserve(CHUNK_SIZE / sizeof(int));

        int val;
        while (buffer.size() < buffer.capacity() && ifs.read(reinterpret_cast<char *>(&val), sizeof(val)))
        {
            buffer.push_back(val);
        }
        if (buffer.empty()) break;

        futures.push_back(pool.enqueue([buffer = std::move(buffer), file, idx]() mutable
                                       {
            std::sort(buffer.begin(), buffer.end());
            return writeSortedChunk(file, idx, buffer); }));
        ++idx;
    }

    // 等待块排序完成
    std::vector<std::string> sortedParts;
    for (auto &f : futures) sortedParts.push_back(f.get());

    // 最终排序文件名
    std::string outputFile = file + ".sorted";

    // 多路归并
    mergeFiles(sortedParts, outputFile);

    // 删除临时块
    for (auto &p : sortedParts) std::remove(p.c_str());

    // 输出排序后前 100 个元素
    std::ifstream sortedFile(outputFile, std::ios::binary);
    std::vector<int> sortedData;
    int tmp;
    while (sortedFile.read(reinterpret_cast<char *>(&tmp), sizeof(tmp)) && sortedData.size() < 100)
        sortedData.push_back(tmp);
    std::cout << "Sorted data (first 100): ";
    for (auto x : sortedData) std::cout << x << " ";
    std::cout << std::endl;
}


int main()
{
    const std::string testFile = "test.bin";

    // 生成随机文件
    auto before = std::chrono::high_resolution_clock::now();
    LRandom::genRandomFile(testFile, 0, 1000000, 5000000);
    auto now = std::chrono::high_resolution_clock::now();

    std::cout << "Random Data generated in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(now - before).count()
              << " ms.\n";

    // 创建线程池
    LThreadPool pool(10);

    // 外部排序
    before = std::chrono::high_resolution_clock::now();
    externalSort(testFile, pool);
    now = std::chrono::high_resolution_clock::now();

    std::cout << "Sort completed in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(now - before).count()
              << " ms.\n";


    return 0;
}
