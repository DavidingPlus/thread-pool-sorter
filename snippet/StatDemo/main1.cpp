/**
 * @file main.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 平时作业二。自学 stat 等函数，获取文件元数据信息。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include <iostream>
#include <string>

#include <sys/stat.h>


int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::cerr << "usage: " << argv[0] << " <filePath>" << std::endl;


        return EXIT_FAILURE;
    }


    const char *filePath = argv[1];

    struct stat fileStat;
    int res = stat(filePath, &fileStat);
    if (-1 == res)
    {
        perror("stat");


        return EXIT_FAILURE;
    }

    // 文件路径。
    std::cout << "文件路径: " << filePath << std::endl;

    // 文件大小。
    std::cout << "文件大小: " << fileStat.st_size << std::endl;

    // 文件类型判断。
    std::cout << "文件 st_mode 值: " << fileStat.st_mode << std::endl;

    if (S_ISREG(fileStat.st_mode))
    {
        std::cout << "文件类型: 普通文件" << std::endl;
    }
    else if (S_ISDIR(fileStat.st_mode))
    {
        std::cout << "文件类型: 目录" << std::endl;
    }
    else if (S_ISLNK(fileStat.st_mode))
    {
        std::cout << "文件类型: 符号链接" << std::endl;
    }
    else
    {
        std::cout << "文件类型: 其他" << std::endl;
    }

    // 文件权限。
    std::cout << "文件权限（八进制）: " << std::oct << (fileStat.st_mode & 0777) << std::dec << std::endl;

    // 所有者与组。
    std::cout << "文件所有者 UID: " << fileStat.st_uid << std::endl;
    std::cout << "文件所属组 GID: " << fileStat.st_gid << std::endl;

    // 时间信息。
    std::cout << "文件最后访问时间: " << ctime(&fileStat.st_atime);
    std::cout << "文件最后修改时间: " << ctime(&fileStat.st_mtime);
    std::cout << "文件最后状态更改时间: " << ctime(&fileStat.st_ctime);


    return 0;
}
