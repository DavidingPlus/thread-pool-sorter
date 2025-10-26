/**
 * @file main.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 平时作业二。自学 stat 等函数，模拟实现 ls -l 命令。
 * @note 运行结果类似：-rwxrwxrwx 1 lzx0626 lzx0626 107 Jul 15 09:40 ls-l.cpp
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include <iostream>
#include <string>
#include <ctime>

#ifdef L_OS_LINUX
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "lglobalmacros.h"


int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::cout << "usage: " << argv[0] << " <filePath>" << std::endl;
        return -1;
    }


#ifdef L_OS_LINUX

    // 通过 stat() 函数获取文件的信息。
    struct stat statbuf;

    int res = stat(argv[1], &statbuf);
    if (-1 == res)
    {
        perror("stat");


        return EXIT_FAILURE;
    }

    // 获取文件类型和文件权限 st_mode 变量。
    std::string perms; // 保存文件类型和权限的字符串。
    mode_t mode = statbuf.st_mode;

    // 获得文件类型和掩码 -S_IFMT 相与。
    switch (mode & S_IFMT)
    {
        case S_IFSOCK:
            perms.append("s");
            break;
        case S_IFLNK:
            perms.append("1");
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
    nlink_t linkNum = statbuf.st_nlink;

    // 文件所有者。
    // 这个函数可以通过用户 uid 获得用户名称。
    std::string user = getpwuid(statbuf.st_uid)->pw_name;

    // 文件所在组。
    // 这个函数通过组 gid 获得名称。
    std::string group = getgrgid(statbuf.st_gid)->gr_name;

    // 文件大小。
    off_t size = statbuf.st_size;

    // 获取修改时间。
    // ctime() 函数可以将时间差值转化为本地时间。
    std::string mtime(ctime(&statbuf.st_mtime));
    // 这个时间格式化之后回车换行了，将其去掉。
    mtime.pop_back();

    // 输出。
    std::cout << perms.c_str() << " " << linkNum << " " << user.c_str() << " " << group.c_str() << " " << size << " " << mtime.c_str() << " " << argv[1] << std::endl;

#endif


    return 0;
}
