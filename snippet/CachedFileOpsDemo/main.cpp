/**
 * @file main.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 平时作业一。带缓存的文件操作类的测试主程序。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include "lcachedfile.h"
#include "lutil.h"

#include <iostream>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>


int main()
{
    const std::string filePath = LUtil::executableDirectory() + "test.txt";
    LCachedFile f;

    // 打开或创建文件。
    f.open(filePath, O_RDWR | O_TRUNC | O_CREAT, 0644);

    const char *s = "Hello World";
    // 写入到缓存。
    ssize_t wn = f.write(s, std::strlen(s));
    std::cout << "write bytes: " << wn << std::endl;

    // 回到文件头。
    f.lseek(0, SEEK_SET);
    char buf[128] = {0};

    // 从缓存读。这个时候文件中应该还没有数据。
    ssize_t rn = f.read(buf, sizeof(buf) - 1);
    std::cout << "read bytes: " << rn << ", content: \"" << buf << "\"" << std::endl;

    sleep(5);

    // 强制 flush 并关闭。
    f.close();
    std::cout << "closed and flushed." << std::endl;

    sleep(5);

    std::remove(filePath.c_str());


    return 0;
}
