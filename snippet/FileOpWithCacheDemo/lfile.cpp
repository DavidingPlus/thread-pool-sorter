/**
 * @file lfile.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 带缓存的文件操作类源文件。
 */

#include "lfile.h"
#include "lcachemanager.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>


LFile::~LFile()
{
    close(); // 析构时确保关闭
}

void LFile::open(const std::string &filePath, int flags, mode_t mode)
{
    if (m_fd != -1)
    {
        close(); // 已打开则先关闭
    }

    int fd;
    if (flags & O_CREAT)
    {
        fd = ::open(filePath.c_str(), flags, mode); // 使用 mode
    }
    else
    {
        fd = ::open(filePath.c_str(), flags); // 不带 mode
    }

    if (fd < 0)
    {
        throw std::runtime_error("open failed"); // 上层可捕获
    }

    m_fd = fd;
    m_path = filePath;
    m_offset = 0;
    LCacheManager::instance().addFile(m_path, m_fd); // 注册到全局缓存管理器
}

void LFile::close()
{
    if (m_fd == -1) return;

    LCacheManager::instance().flush(m_path);     // flush 文件脏块
    LCacheManager::instance().closeFile(m_path); // 注销并 close(fd)
    m_fd = -1;
    m_path.clear();
    m_offset = 0;
}

ssize_t LFile::read(void *buf, size_t count)
{
    if (m_fd == -1) return -1;
    return LCacheManager::instance().read(m_path, buf, count, m_offset);
}

ssize_t LFile::write(const void *buf, size_t count)
{
    if (m_fd == -1) return -1;
    return LCacheManager::instance().write(m_path, buf, count, m_offset);
}

off_t LFile::lseek(off_t offset, int whence)
{
    if (m_fd == -1) return -1;

    if (whence == SEEK_SET)
    {
        m_offset = offset;
    }
    else if (whence == SEEK_CUR)
    {
        m_offset += offset;
    }
    else if (whence == SEEK_END)
    {
        off_t r = ::lseek(m_fd, offset, SEEK_END);
        if (r >= 0) m_offset = r;
    }
    return m_offset;
}
