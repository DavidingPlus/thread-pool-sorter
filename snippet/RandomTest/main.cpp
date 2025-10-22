#include <iostream>
#include <fstream>

#include "lrandom.h"


int main()
{
    for (int i = 0; i < 5; ++i) std::cout << LRandom::genRandomNumber(1, 100) << ' ';
    std::cout << std::endl;

    std::cout << std::endl;

    std::vector<int> vec = LRandom::genRandomVector(1, 100, 10000);
    for (int i = 0; i < vec.size(); ++i) std::cout << vec[i] << ' ';
    std::cout << std::endl;

    std::cout << std::endl;

    const std::string filePath = "text.txt";

    LRandom::genRandomFile(filePath, 1, 100, 10);

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

    // 打印前 10 个元素做检查。
    std::cout << "data size: " << data.size() << "\n";
    for (size_t i = 0; i < data.size(); ++i) std::cout << data[i] << " ";
    std::cout << std::endl;


    return 0;
}
