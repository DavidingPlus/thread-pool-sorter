#include <iostream>
#include <fstream>

#include "lrandom.h"


int main()
{
    const std::string filePath = "text.txt";

    LRandom::genRandomFile(filePath, 1, 1000000, 5000000);

    std::ifstream file(filePath, std::ios::binary | std::ios::in);
    if (!file) throw std::runtime_error("Failed to open file: " + filePath);

    // 获取文件大小。
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // 计算数据个数。
    size_t numElements = fileSize / sizeof(int);

    // 读取文件到 vector。
    std::vector<int> data(numElements);

    if (!file.read(reinterpret_cast<char *>(data.data()), fileSize)) throw std::runtime_error("Error when reading file");

    file.close();

    // 打印元素。
    std::cout << "data size: " << data.size() << std::endl;
    // for (size_t i = 0; i < data.size(); ++i) std::cout << data[i] << " ";
    // std::cout << std::endl;


    return 0;
}
