/**
 * @file lfile.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 平时作业一。带缓存的文件操作类。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include "lfile.h"

#include <cstring>

#include <unistd.h>


void LFile::lopen(const std::string &filePath, int flags, mode_t mode)
{
    if (0 == mode)
    {
        m_fd = open(filePath.c_str(), flags);
    }
    else
    {
        m_fd = open(filePath.c_str(), flags, mode);
    }

    if (-1 == m_fd) throw std::runtime_error("lopen: " + std::string(std::strerror(errno)));
}

void LFile::lclose()
{
    if (-1 == m_fd) return;

    close(m_fd);

    m_fd = -1;
}

ssize_t LFile::lread(void *buf, size_t count)
{
    if (-1 == m_fd) throw std::runtime_error("lread: cannot read an invalid file");


    return read(m_fd, buf, count);
}

ssize_t LFile::lwrite(const void *buf, size_t count)
{
    if (-1 == m_fd) throw std::runtime_error("lwrite: cannot write an invalid file");


    return write(m_fd, buf, count);
}

off_t LFile::llseek(off_t offset, int whence)
{
    if (-1 == m_fd) throw std::runtime_error("llseek: cannot lseek an invalid file");


    return lseek(m_fd, offset, whence);
}
