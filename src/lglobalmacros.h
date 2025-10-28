/**
 * @file lglobalmacros.h
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 全局宏定义头文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#ifndef _LGLOBALMACROS_H_
#define _LGLOBALMACROS_H_


#ifdef _WIN32
#define L_OS_WIN32
#elif __unix__
#define L_OS_LINUX
#else
#define L_OS_UNKNOWN
#endif


#define L_CLASS_NONCOPYABLE(ClassName)                     \
                                                           \
private:                                                   \
                                                           \
    ClassName(const ClassName &other) = delete;            \
    ClassName(ClassName &&other) = delete;                 \
    ClassName &operator=(const ClassName &other) = delete; \
    ClassName &operator=(ClassName &&other) = delete;


#endif
