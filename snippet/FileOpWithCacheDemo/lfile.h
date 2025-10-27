/**
 * @file lfile.h
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 平时作业一。带缓存的文件操作类。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#ifndef _LFILE_H_
#define _LFILE_H_

#include <iostream>
#include <string>

#include <fcntl.h>


class LFile
{

public:

    LFile() = default;

    virtual ~LFile() { close(); }

    void open(const std::string &filePath, int flags, mode_t mode = 0);

    void close();

    ssize_t read(void *buf, size_t count);

    ssize_t write(const void *buf, size_t count);

    off_t lseek(off_t offset, int whence);


private:

    int m_fd = -1;
};


#endif
