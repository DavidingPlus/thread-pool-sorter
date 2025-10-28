/**
 * @file lutil.h
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 一些工具函数的头文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#ifndef _LUTIL_H_
#define _LUTIL_H_

#include <string>


namespace LUtil
{
    /**
     * @brief 返回可执行文件完整路径。
     * @return 可执行文件路径。
     */
    std::string executableFullPath();

    /**
     * @brief 返回可执行文件所在目录的完整路径。
     * @return 可执行文件所在目录路径。
     */
    std::string executableDirectory();
}


#endif
