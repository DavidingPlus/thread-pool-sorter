/**
 * @file lfile.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 带缓存的文件操作类源文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include "lfile.h"

#include <cstring>

#include <unistd.h>


void LFile::open(const std::string &filePath, int flags, mode_t mode)
{
    if (0 == mode)
    {
        m_fd = ::open(filePath.c_str(), flags);
    }
    else
    {
        m_fd = ::open(filePath.c_str(), flags, mode);
    }

    if (-1 == m_fd) throw std::runtime_error("open: " + std::string(std::strerror(errno)));
}

void LFile::close()
{
    if (-1 == m_fd) return;

    ::close(m_fd);

    m_fd = -1;
}

ssize_t LFile::read(void *buf, size_t count)
{
    if (-1 == m_fd) throw std::runtime_error("read: cannot read an invalid file");


    return ::read(m_fd, buf, count);
}

ssize_t LFile::write(const void *buf, size_t count)
{
    if (-1 == m_fd) throw std::runtime_error("write: cannot write an invalid file");


    return ::write(m_fd, buf, count);
}

off_t LFile::lseek(off_t offset, int whence)
{
    if (-1 == m_fd) throw std::runtime_error("lseek: cannot lseek an invalid file");


    return ::lseek(m_fd, offset, whence);
}
