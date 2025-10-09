/**
 * @file lthreadpool.h
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 线程池类头文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#ifndef _LTHREADPOOL_H_
#define _LTHREADPOOL_H_

#include <thread>
#include <string>


class LThreadPool
{

public:

    // TODO 测试接口，后续删掉。
    static std::string foo() { return std::string("hello world"); }

    std::pair<int, int> gee(int first, int second) const { return std::pair<int, int>{first, second}; }
};


#endif
