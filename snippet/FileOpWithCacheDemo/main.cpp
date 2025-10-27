/**
 * @file main.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 平时作业一。带缓存的文件操作类的测试主程序。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include <iostream>

#include "lfile.h"


constexpr int maxBufferSize = 1024;


int main()
{
    LFile file;

    // 创建文件。
    file.open("test.txt", O_CREAT, 0664);
    file.close();

    // 读取刚才创建的文件。
    file.open("test.txt", O_RDWR);

    // 写文件。
    const std::string s = "hello world";
    file.write(s.c_str(), s.size());

    // lseek
    file.lseek(0, SEEK_SET);

    // 读文件。
    char buf[maxBufferSize] = {0};
    file.read(buf, sizeof(buf));
    std::cout << buf << std::endl;

    // 关闭文件。
    file.close();


    return 0;
}
