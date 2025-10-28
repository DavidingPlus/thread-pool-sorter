/**
 * @file lfile.h
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 带缓存的文件操作类头文件。
 * @note 调用全局 LCacheManager 提供缓存读写。提供 open/read/write/lseek/close 接口。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 */

#ifndef _LFILE_H_
#define _LFILE_H_

#include <string>

#include <sys/types.h>


/**
 * @class LFile
 * @brief 带缓存的文件操作系统类。
 */
class LFile
{

public:

    /**
     * @brief 默认构造函数。
     */
    LFile() = default;

    /**
     * @brief 析构函数。
     */
    virtual ~LFile() { close(); }

    /**
     * @brief 打开文件，并在全局缓存管理器 LCacheManager 中注册。
     * @param filePath 文件路径。
     * @param flags 文件打开标志（如 O_RDONLY, O_WRONLY, O_RDWR 等）。
     * @param mode 文件权限，仅在创建新文件时有效，默认为 0。
     */
    void open(const std::string &filePath, int flags, mode_t mode = 0);

    /**
     * @brief 关闭文件，在 LCacheManager 中注销并将所有脏块写回磁盘。
     */
    void close();

    /**
     * @brief 从文件中读取数据。
     * @param buf 指向数据缓冲区的指针。
     * @param count 读取字节数。
     * @return 实际读取的字节数，返回 -1 表示出错。
     */
    ssize_t read(void *buf, size_t count);

    /**
     * @brief 将数据写入文件。
     * @param buf 指向数据缓冲区的指针。
     * @param count 写入字节数。
     * @return 实际写入的字节数，返回 -1 表示出错。
     * @details 数据会先写入缓存，延迟写回磁盘。
     */
    ssize_t write(const void *buf, size_t count);

    /**
     * @brief 修改文件内部偏移量。
     * @param offset 偏移量。
     * @param whence 偏移起点，取值同系统 lseek，如 SEEK_SET, SEEK_CUR, SEEK_END 等。
     * @return 新的文件内部偏移量，失败返回 -1。
     */
    off_t lseek(off_t offset, int whence);

    /**
     * @brief 获取当前文件内部偏移量。
     * @return 当前偏移量。
     */
    off_t tell() const { return m_offset; }


private:

    /**
     * @brief 底层文件描述符。
     */
    int m_fd = -1;

    /**
     * @brief 文件路径，用作缓存的 Key。
     */
    std::string m_path;

    /**
     * @brief 文件偏移量。
     */
    off_t m_offset = 0;
};


#endif
