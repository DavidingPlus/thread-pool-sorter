/**
 * @file lutil.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 一些工具函数的源文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include "lutil.h"

#include "lglobalmacros.h"

#include <string>
#include <stdexcept>

#ifdef L_OS_WIN32
#include <shlwapi.h>
#endif

#ifdef L_OS_LINUX
#include <unistd.h>
#include <dirent.h>
#endif


std::string LUtil::executableFullPath()
{
#ifdef L_OS_WIN32

    TCHAR buffer[MAX_PATH] = {0};
    if (0 == GetModuleFileName(nullptr, buffer, MAX_PATH)) throw std::runtime_error(std ::string("无法获取可执行文件完整路径，错误代码：") + std::to_string(GetLastError()));


    return std::string(buffer);

#endif

#ifdef L_OS_LINUX

    char buffer[PATH_MAX] = {0};
    if (readlink("/proc/self/exe", buffer, PATH_MAX) <= 0) throw std::runtime_error(std ::string("无法获取可执行文件完整路径，错误代码：") + std::to_string(errno));


    return std::string(buffer);

#endif


    return {};
}

std::string LUtil::executableDirectory()
{
    std::string fullPath = executableFullPath();

#ifdef L_OS_WIN32

    char drive[_MAX_DRIVE] = {0};
    char dir[_MAX_DIR] = {0};
    _splitpath_s(fullPath.c_str(), drive, sizeof(drive), dir, sizeof(dir), nullptr, 0, nullptr, 0);


    return std::string(drive) + std::string(dir);

#endif

#ifdef L_OS_LINUX

    size_t pos = fullPath.find_last_of('/');
    if (std::string::npos == pos) return ".";


    return fullPath.substr(0, 1 + pos);

#endif


    return {};
}
