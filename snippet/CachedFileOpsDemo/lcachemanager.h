/**
 * @file lcachemanager.h
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 全局文件缓存管理器头文件。
 *
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 */

#ifndef _LCACHEMANAGER_H_
#define _LCACHEMANAGER_H_

#include "lglobalmacros.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <list>
#include <mutex>

#ifdef L_OS_LINUX
#include <sys/types.h>
#endif


/**
 * @class LCacheManager
 * @brief 全局文件缓存管理器类。
 * @details  LCacheManager 是一个单例类，管理所有文件的缓存块。
 * 1. 每个文件按路径维护 FileCache（包含多个 CacheBlock）。
 * 2. 以块大小 m_blockSize 为单位。
 * 3. 简单 LRU（基于块 key 的双向链表）用于控制最大缓存块数。
 * 4. 线程安全，对外接口加 mutex。
 */
class LCacheManager
{
    L_CLASS_NONCOPYABLE(LCacheManager)

#ifdef L_OS_LINUX

public:

    /**
     * @brief 获取 LCacheManager 单例。
     * @return LCacheManager 引用。
     */
    static LCacheManager &instance();

    /**
     * @brief 析构函数。
     * @note 自动调用 flushAll，确保所有脏块落盘。
     */
    ~LCacheManager();

    /**
     * @brief 打开并注册文件到缓存管理。LCachedFile 在 open 时调用。
     * @param filePath 文件路径。
     * @param fd 已打开的文件描述符。
     */
    void addFile(const std::string &filePath, int fd);

    /**
     * @brief 关闭并注销文件。会 flush 对应文件所有脏块并关闭文件描述符。
     * @param filePath 文件路径。
     */
    void closeFile(const std::string &filePath);

    /**
     * @brief 从文件读取数据。
     * @param filePath 文件路径。
     * @param buf 读取缓冲区。
     * @param count 读取字节数。
     * @param offset 调用者当前偏移，函数会更新该偏移。
     * @return 实际读取字节数，失败返回 -1。
     */
    ssize_t read(const std::string &filePath, void *buf, size_t count, off_t &offset);

    /**
     * @brief 写入数据到文件缓存。写入数据会标记为脏块，延迟写回磁盘。
     * @param filePath 文件路径。
     * @param buf 写入数据缓冲区。
     * @param count 写入字节数。
     * @param offset 调用者当前偏移，写入缓存后会更新。
     * @return 实际写入到缓存的字节数（可能小于 count）。
     * @note
     */
    ssize_t write(const std::string &filePath, const void *buf, size_t count, off_t &offset);

    /**
     * @brief 强制将某文件的所有脏块写回磁盘。
     * @param filePath 文件路径。
     */
    void flush(const std::string &filePath);

    /**
     * @brief 强制将所有文件的脏块写回磁盘。
     */
    void flushAll();

    /**
     * @brief 设置全局缓存块上限。。
     * @param maxBlocks 最大缓存块数量。
     */
    void setMaxBlocks(size_t maxBlocks);


private:

    /**
     * @struct CacheBlock
     * @brief 文件缓存块。
     */
    struct CacheBlock
    {
        /**
         * @brief 块在文件中的起始偏移。
         */
        off_t baseOffset;

        /**
         * @brief 块内有效数据长度。
         */
        size_t dataSize;

        /**
         * @brief 是否被修改过需要写回。
         */
        bool dirty;

        /**
         * @brief 缓存块数据缓冲区。
         */
        std::vector<char> data;
    };

    /**
     * @struct FileCache
     * @brief 每个文件对应的缓存集合。
     */
    struct FileCache
    {
        /**
         * @brief 文件描述符。
         */
        int fd = -1;

        /**
         * @brief 缓存块集合。
         * @note Key 是块在文件中的起始偏移。
         */
        std::unordered_map<off_t, CacheBlock> blocks;
    };


private:

    /**
     * @brief 单例模式下的私有构造函数。
     */
    LCacheManager();


    /**
     * @brief 确保块存在并返回引用（必要时从磁盘加载）。
     * @param filePath 文件路径。
     * @param blockBase 块起始偏移。
     * @return CacheBlock 引用。
     */
    CacheBlock &ensureBlock(const std::string &filePath, off_t blockBase);

    /**
     * @brief 更新 LRU，将块移动到 front 位置。
     * @param filePath 文件路径。
     * @param blockBase 块起始偏移。
     */
    void updateLRU(const std::string &filePath, off_t blockBase);

    /**
     * @brief 当缓存超上限时，驱逐最久未用块。
     */
    void evictIfNeeded();

    /**
     * @brief 将单个块写回磁盘（不加锁）。
     * @param fc 文件缓存。
     * @param blockBase 块起始偏移。
     */
    void writeBlockToDisk(FileCache &fc, off_t blockBase);


private:

    /**
     * @brief LRU 键：文件路径 + 块起始偏移。
     */
    using LRUKey = std::pair<std::string, off_t>;

    /**
     * @brief LRU 链表，front 是最近使用，back 是最久未用。
     */
    std::list<LRUKey> m_lruList;

    /**
     * @brief 文件缓存集合。
     */
    std::unordered_map<std::string, FileCache> m_fileCaches;

    /**
     * @brief LRU 哈希表，用于快速定位。
     */
    std::unordered_map<std::string, std::unordered_map<off_t, std::list<LRUKey>::iterator>> m_lruIters;

    /**
     * @brief 块大小。
     */
    size_t m_blockSize;

    /**
     * @brief 最大缓存块数。
     */
    size_t m_maxBlocks;

    /**
     * @brief 当前的缓存块数。
     */
    size_t m_curBlocks;

    /**
     * @brief 互斥锁。用于线程保护。
     */
    std::mutex m_mutex;

#endif
};


#endif
