/**
 * @file lfile.h
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 带缓存的文件操作类头文件。
 * @note 调用全局 LCacheManager 提供缓存读写。提供 open/read/write/lseek/close 接口。
 */

#ifndef _LFILE_H_
#define _LFILE_H_

#include <string>

#include <sys/types.h>


class LFile
{

public:

    LFile() = default;
    ~LFile();

    // 打开文件（会在 LCacheManager 中注册）
    void open(const std::string &filePath, int flags, mode_t mode = 0);

    // 关闭文件（会在 LCacheManager 中注销并 flush）
    void close();

    // 读写（调用 LCacheManager）
    ssize_t read(void *buf, size_t count);
    ssize_t write(const void *buf, size_t count);

    // lseek 修改内部偏移（返回新的偏移）
    off_t lseek(off_t offset, int whence);

    // 获取当前内部偏移
    off_t tell() const { return m_offset; }


private:

    int m_fd = -1;      // 底层 fd
    std::string m_path; // 文件路径（作为 cache key）
    off_t m_offset = 0; // 当前偏移（由 lseek/read/write 更新）
};


#endif
