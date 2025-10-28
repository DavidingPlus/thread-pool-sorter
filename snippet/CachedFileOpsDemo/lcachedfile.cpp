/**
 * @file lcachedfile.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 带缓存的文件操作类源文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 */

#include "lcachedfile.h"

#include "lcachemanager.h"

#include <stdexcept>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


void LCachedFile::open(const std::string &filePath, int flags, mode_t mode)
{
    if (-1 == m_fd) close();


    // 使用 mode。
    if (flags & O_CREAT)
    {
        m_fd = ::open(filePath.c_str(), flags, mode);
    }
    // 不带 mode。
    else
    {
        m_fd = ::open(filePath.c_str(), flags);
    }

    if (-1 == m_fd)
    {
        throw std::runtime_error("open: " + std::string(std::strerror(errno)));
    }

    m_path = filePath;
    m_offset = 0;

    // 注册到全局缓存管理器。
    LCacheManager::instance().addFile(m_path, m_fd);
}

void LCachedFile::close()
{
    if (-1 == m_fd) return;

    // 刷新缓存中的脏块。
    LCacheManager::instance().flush(m_path);
    // 关闭并销毁文件描述符。
    LCacheManager::instance().closeFile(m_path);

    m_fd = -1;
    m_path.clear();
    m_offset = 0;
}

ssize_t LCachedFile::read(void *buf, size_t count)
{
    if (-1 == m_fd) return -1;


    return LCacheManager::instance().read(m_path, buf, count, m_offset);
}

ssize_t LCachedFile::write(const void *buf, size_t count)
{
    if (-1 == m_fd) return -1;


    return LCacheManager::instance().write(m_path, buf, count, m_offset);
}

off_t LCachedFile::lseek(off_t offset, int whence)
{
    if (m_fd < 0) return -1;


    // 需要注意的是，缓存中的文件偏移量和实际文件偏移量是两个东西，因为我们做了缓存，这个偏移量是针对缓存的。当然在 LCacheManager 中会有和实际文件偏移量的同步操作。
    off_t newOffset = -1;
    switch (whence)
    {
        case SEEK_SET:
        {
            if (offset < 0) return -1;
            newOffset = offset;
            break;
        }

        case SEEK_CUR:
        {
            if (m_offset + offset < 0) return -1;
            newOffset = m_offset + offset;
            break;
        }

        case SEEK_END:
        {
            // 获取文件大小。
            struct stat st;
            if (fstat(m_fd, &st) < 0) return -1;
            if (st.st_size + offset < 0) return -1;
            newOffset = st.st_size + offset;
            break;
        }

        default:
            return -1;
    }

    m_offset = newOffset;


    return m_offset;
}
