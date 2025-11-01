/**
 * @file main.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 平时作业二。自学 stat 等函数，模拟实现 ls -l 命令。
 * @note 运行结果类似：-rwxrwxrwx 1 lzx0626 lzx0626 107 Jul 15 09:40 ls-l.cpp
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include "lglobalmacros.h"

#include <iostream>
#include <string>
#include <ctime>

#ifdef L_OS_LINUX
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#endif


#ifdef L_OS_LINUX

void printStatInfo(const struct stat &statBuf, const std::string &fileName)
{
    // 获取文件类型和文件权限 st_mode 变量。
    std::string perms; // 保存文件类型和权限的字符串。
    mode_t mode = statBuf.st_mode;

    // 获得文件类型和掩码 -S_IFMT 相与。
    switch (mode & S_IFMT)
    {
        case S_IFSOCK:
            perms.append("s");
            break;
        case S_IFLNK:
            perms.append("l");
            break;
        case S_IFREG:
            perms.append("-");
            break;
        case S_IFBLK:
            perms.append("b");
            break;
        case S_IFDIR:
            perms.append("d");
            break;
        case S_IFCHR:
            perms.append("c");
            break;
        case S_IFIFO:
            perms.append("p");
            break;
        default:
            perms.append("?");
            break;
    }

    // 判断文件访问权限 Users Group Others。
    // Users
    perms.append((mode & S_IRUSR) ? "r" : "-");
    perms.append((mode & S_IWUSR) ? "w" : "-");
    perms.append((mode & S_IXUSR) ? "x" : "-");
    // Group
    perms.append((mode & S_IRGRP) ? "r" : "-");
    perms.append((mode & S_IWGRP) ? "w" : "-");
    perms.append((mode & S_IXGRP) ? "x" : "-");
    // Others
    perms.append((mode & S_IROTH) ? "r" : "-");
    perms.append((mode & S_IWOTH) ? "w" : "-");
    perms.append((mode & S_IXOTH) ? "x" : "-");

    // 获取硬连接数。
    nlink_t linkNum = statBuf.st_nlink;

    // 文件所有者。
    // 这个函数可以通过用户 uid 获得用户名称。
    std::string user = getpwuid(statBuf.st_uid)->pw_name;

    // 文件所在组。
    // 这个函数通过组 gid 获得名称。
    std::string group = getgrgid(statBuf.st_gid)->gr_name;

    // 文件大小。
    off_t size = statBuf.st_size;

    // 获取修改时间。
    // ctime() 函数可以将时间差值转化为本地时间。
    std::string mtime(ctime(&statBuf.st_mtime));
    // 这个时间格式化之后回车换行了，将其去掉。
    mtime.pop_back();

    // 输出。
    std::cout << perms.c_str() << " " << linkNum << " " << user.c_str() << " " << group.c_str() << " " << size << " " << mtime.c_str() << " " << fileName << std::endl;
}

#endif


int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::cout << "usage: " << argv[0] << " <filePath>" << std::endl;
        return -1;
    }


#ifdef L_OS_LINUX

    // 通过 stat() 函数获取文件的信息。
    struct stat statBuf;

    int res = stat(argv[1], &statBuf);
    if (-1 == res)
    {
        perror("stat");


        return EXIT_FAILURE;
    }

    // 如果是文件，直接输出信息。
    if (S_ISREG(statBuf.st_mode))
    {
        printStatInfo(statBuf, argv[1]);
    }
    // 如果是目录，递归遍历当前目录的文件，并输出信息。
    else if (S_ISDIR(statBuf.st_mode))
    {
        DIR *pDir = opendir(argv[1]);
        if (!pDir)
        {
            perror("opendir");


            return EXIT_FAILURE;
        }

        struct dirent *pDirent = nullptr;
        while (pDirent = readdir(pDir))
        {
            std::string name(pDirent->d_name);

            res = stat((std::string(argv[1]) + "/" + name).c_str(), &statBuf);
            if (-1 == res)
            {
                perror("stat");


                return EXIT_FAILURE;
            }

            printStatInfo(statBuf, name);
        }
    }
    // 其他类型不支持。
    else
    {
        std::cerr << "暂不支持除了普通文件和目录以外的其他类型。\n";
    }


#endif


    return 0;
}
