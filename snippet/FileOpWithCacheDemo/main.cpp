/**
 * @file main.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 平时作业一。带缓存的文件操作类的测试主程序。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include "lfile.h"

#include <iostream>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>


int main()
{

    LFile f;
    f.open("test.dat", O_RDWR | O_TRUNC | O_CREAT, 0644); // 打开或创建文件

    const char *s = "Hello block-cache world! This is a test string to span multiple blocks if needed.";
    ssize_t wn = f.write(s, std::strlen(s)); // 写入（写入到缓存）
    std::cout << "write bytes: " << wn << std::endl;

    f.lseek(0, SEEK_SET); // 回到文件头
    char buf[128] = {0};
    ssize_t rn = f.read(buf, sizeof(buf) - 1); // 从缓存读
    std::cout << "read bytes: " << rn << ", content: [" << buf << "]" << std::endl;

    sleep(5);

    // 强制 flush 并关闭
    f.close();
    std::cout << "closed and flushed." << std::endl;

    sleep(5);

    std::remove("test.dat");


    return 0;
}
