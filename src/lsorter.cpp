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
    if (!pool) throw std::runtime_error("Pointer pool is a nullptr.");
}

LSorter::~LSorter()
{
    if (m_pool) m_pool = nullptr;
}

void LSorter::run(const std::string &filePath)
{
    // 函数执行逻辑：
    // 1. 打开待排序的二进制文件。
    // 2.（可选）读取文件前 100 个元素并输出，用于原始数据调试。
    // 3. 分块读取文件数据，每块大小为 m_chunkSize，将每块数据提交给线程池进行排序。每个任务完成后生成一个临时排序文件。
    // 4. 等待所有分块排序任务完成，收集生成的临时文件路径。
    // 5. 多轮 k 路归并：
    //   - 每轮将临时文件分为若干组，每组最多 m_k 个文件。
    //   - 对每组文件，若只有一个文件直接进入下一轮，否则提交归并任务到线程池。
    //   - 每轮归并完成后生成新临时文件，旧文件被删除。
    //   - 重复直到只剩下一个最终文件。
    // 6. 将最终文件重命名为原文件名 + ".sorted"。
    // 7. （可选）输出最终排序文件前 100 个元素，用于排序结果验证。

    // 打开文件。
    std::ifstream ifs(filePath, std::ios::binary);
    if (!ifs) return;

    // 存储分块排序任务的 future。
    std::vector<std::future<std::string>> futures;
    int index = 0;
    std::vector<int> originalData;

    // （可选）读取原始数据前 100 个元素。
    int val;
    while (ifs.read(reinterpret_cast<char *>(&val), sizeof(val)) && originalData.size() < 100) originalData.push_back(val);

    std::cout << "Original data (first 100): ";
    for (auto x : originalData) std::cout << x << " ";
    std::cout << std::endl;

    // 重置文件流以便重新读取。
    ifs.clear();
    ifs.seekg(0);

    // 分块读取文件并提交排序任务。
    while (!ifs.eof())
    {
        // 分配缓冲区。
        std::vector<int> buffer;
        buffer.reserve(m_chunkSize / sizeof(int));

        while (buffer.size() < buffer.capacity() && ifs.read(reinterpret_cast<char *>(&val), sizeof(val))) buffer.push_back(val);
        if (buffer.empty()) break;

        // 提交排序任务到线程池。
        futures.push_back(m_pool->enqueue([buffer = std::move(buffer), filePath, index, this]() mutable { // 排序内存块并写入临时文件。
            std::sort(buffer.begin(), buffer.end());
            return writeSortedChunk(filePath, index, buffer);
        }));

        ++index;
    }

    // 获取所有排序块生成的临时文件路径。
    std::vector<std::string> sortedinputFiles;
    for (auto &f : futures) sortedinputFiles.push_back(f.get());

    // 多轮 k 路归并。
    int mergeRound = 0;
    while (sortedinputFiles.size() > 1)
    {
        // 存储归并任务的 future。
        std::vector<std::future<std::string>> mergeFutures;
        // 下一轮归并文件列表。
        std::vector<std::string> nextRoundFilePaths;

        for (int i = 0; i < sortedinputFiles.size(); i += m_k)
        {
            std::vector<std::string> group;
            for (int j = i; j < i + m_k && j < sortedinputFiles.size(); ++j) group.push_back(sortedinputFiles[j]);

            if (1 == group.size())
            {
                // 单文件直接加入下一轮。
                nextRoundFilePaths.push_back(group[0]);
            }
            else
            {
                // 提交归并任务到线程池。
                mergeFutures.push_back(m_pool->enqueue([group, mergeRound, i, this]() { //
                    return mergeKFiles(group, mergeRound * 1000 + i);
                }));
            }
        }

        // 获取归并任务结果。
        for (auto &mf : mergeFutures) nextRoundFilePaths.push_back(mf.get());

        // 更新文件列表。
        sortedinputFiles = std::move(nextRoundFilePaths);

        ++mergeRound;
    }

    // 重命名最终归并文件。
    std::string finalFilePath = filePath + ".sorted";
    std::rename(sortedinputFiles[0].c_str(), finalFilePath.c_str());

    // （可选）输出最终排序前 100 个元素。
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

std::string LSorter::mergeKFiles(const std::vector<std::string> &filePaths, unsigned int index)
{
    // 函数执行逻辑：
    // 1. 如果输入文件列表为空，直接返回空字符串。
    // 2. 如果输入文件列表只有一个文件，直接返回该文件路径。
    // 3. 定义 Node 结构体，存储当前文件的元素值和对应文件索引，用于最小堆归并。
    // 4. 打开所有输入文件的二进制流。
    // 5. 使用 std::priority_queue 构建小根堆，将每个文件的首元素压入堆中。
    // 6. 迭代堆：
    //    - 取出堆顶最小元素，写入输出文件。
    //    - 从该元素所在的文件读取下一个值，若存在则继续压入堆。
    // 7. 所有元素处理完毕后关闭输入流，并删除原始临时文件。
    // 8. 返回生成的归并临时文件路径。

    // 处理特殊情况。
    if (filePaths.empty()) return std::string();
    if (1 == filePaths.size()) return filePaths[0];

    // 定义内部 Node 结构体用于堆归并。
    struct Node
    {
        int val;       // 当前元素值。
        int fileindex; // 元素来源文件索引。
        bool operator>(const Node &other) const { return val > other.val; }
    };

    // 打开所有输入文件。
    std::vector<std::ifstream> inputFiles(filePaths.size());
    for (int i = 0; i < filePaths.size(); ++i) inputFiles[i].open(filePaths[i], std::ios::binary);

    // 使用小根堆进行 k 路归并。
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

    // 初始化堆，将每个文件的首元素加入堆。
    for (int i = 0; i < inputFiles.size(); ++i)
    {
        int v;
        if (inputFiles[i].read(reinterpret_cast<char *>(&v), sizeof(v))) pq.push({v, i});
    }

    // 输出文件路径。
    std::string outputFilePath = "tmp_merge_" + std::to_string(index) + ".bin";
    std::ofstream outputFiles(outputFilePath, std::ios::binary);

    // 堆归并。
    while (!pq.empty())
    {
        Node node = pq.top();
        pq.pop();
        // 写入最小元素。
        outputFiles.write(reinterpret_cast<char *>(&node.val), sizeof(node.val));

        // 读取该文件下一个元素并加入堆。
        int v;
        if (inputFiles[node.fileindex].read(reinterpret_cast<char *>(&v), sizeof(v))) pq.push({v, node.fileindex});
    }

    // 关闭输入流并删除源文件。
    for (auto &s : inputFiles) s.close();
    for (const auto &f : filePaths) std::remove(f.c_str());


    return outputFilePath;
}
