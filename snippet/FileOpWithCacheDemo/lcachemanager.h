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

#include <string>
#include <unordered_map>
#include <vector>
#include <list>
#include <mutex>
#include <sys/types.h>


/**
 * @class LCacheManager
 * @brief 全局文件缓存管理器类。
 * @details
 * 单例 LCacheManager::instance()。
 * 每个文件按路径维护 FileCache（包含多个 CacheBlock）。
 * 块大小 BLOCK_SIZE。
 * 简单 LRU（基于块 key 的双向链表）用于控制最大缓存块数。
 * 线程安全，对外接口加 mutex。
 */
class LCacheManager
{

public:

    // 获取单例
    static LCacheManager &instance();

    // 打开/注册文件到缓存管理（LFile 在 open 时调用）
    // path: 文件路径; fd: 已打开的 fd
    void addFile(const std::string &path, int fd);

    // 关闭/注销文件（会 flush 对应文件所有脏块并 close(fd)）
    void closeFile(const std::string &path);

    // 读操作；offset 为调用者当前偏移，函数会更新该 offset（如读了 n 字节则 += n）
    ssize_t read(const std::string &path, void *buf, size_t count, off_t &offset);

    // 写操作；offset 为调用者当前偏移，写入缓存块并标记脏；返回实际写入到缓存的字节数（可能小于 count）
    ssize_t write(const std::string &path, const void *buf, size_t count, off_t &offset);

    // 强制把某文件的所有脏块写回磁盘
    void flush(const std::string &path);

    // 强制把所有文件的脏块写回磁盘
    void flushAll();

    // 设置总缓存块上限（全局），默认为 1024 块
    void setMaxBlocks(size_t maxBlocks);

    // 析构时自动 flushAll
    ~LCacheManager();

private:

    LCacheManager(); // 私有构造（单例）
    LCacheManager(const LCacheManager &) = delete;
    LCacheManager &operator=(const LCacheManager &) = delete;

    // 内部结构：每个块
    struct CacheBlock
    {
        off_t baseOffset;       // 块在文件中的起始偏移（对齐 BLOCK_SIZE）
        size_t dataSize;        // 块内有效数据长度（<= BLOCK_SIZE）
        bool dirty;             // 是否被修改过需要写回
        std::vector<char> data; // BLOCK_SIZE 缓冲区
    };

    // 每个文件的缓存集合
    struct FileCache
    {
        int fd = -1;                                  // 文件描述符
        std::unordered_map<off_t, CacheBlock> blocks; // key = baseOffset
    };

    // LRU 管理 —— 存储全球（path, blockBaseOffset）键的链表
    using LRUKey = std::pair<std::string, off_t>;            // 文件路径 + 块起始偏移
    std::list<LRUKey> m_lruList;                             // front = 最近使用, back = 最久未用
    std::unordered_map<std::string, FileCache> m_fileCaches; // path -> FileCache

    // 快速定位一个块在 LRUList 中的 iterator（key -> iterator）
    std::unordered_map<std::string, std::unordered_map<off_t, std::list<LRUKey>::iterator>> m_lruIters;

    // 参数
    size_t m_blockSize; // BLOCK 大小
    size_t m_maxBlocks; // 最大缓存块数
    size_t m_curBlocks; // 当前缓存块数

    // 线程保护
    std::mutex m_mutex;

    // 内部辅助函数（调用时假定已锁住 mutex）
    CacheBlock &ensureBlock(const std::string &path, off_t blockBase); // 确保块存在并返回引用（可能从磁盘加载）
    void touchLRU(const std::string &path, off_t blockBase);           // 更新 LRU（将块移动到 front）
    void evictIfNeeded();                                              // 当超出上限时驱逐最久未用块
    void writeBlockToDisk(FileCache &fc, off_t blockBase);             // 将单个块写到磁盘（不加锁）
};


#endif
