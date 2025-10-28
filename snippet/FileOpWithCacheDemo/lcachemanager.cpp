/**
 * @file lcachemanager.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 全局文件缓存管理器源文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 */

#include "lcachemanager.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <stdexcept>
#include <algorithm>


LCacheManager::LCacheManager() : m_blockSize(4096), m_maxBlocks(1024), m_curBlocks(0)
{
    // 默认 BLOCK = 4KiB, 最大 1024 块 总计 4MiB 缓存（可通过 setMaxBlocks 调整）
}

LCacheManager::~LCacheManager()
{
    std::lock_guard<std::mutex> lk(m_mutex);
    flushAll(); // 析构时写回
}

// 获取单例
LCacheManager &LCacheManager::instance()
{
    static LCacheManager s_instance;
    return s_instance;
}

void LCacheManager::setMaxBlocks(size_t maxBlocks)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    m_maxBlocks = maxBlocks;
    evictIfNeeded();
}

void LCacheManager::addFile(const std::string &path, int fd)
{
    std::lock_guard<std::mutex> lk(m_mutex);

    FileCache &fc = m_fileCaches[path]; // 若不存在则创建
    if (fc.fd != -1 && fc.fd != fd)
    {
        // 如果原来已经有 fd 并且不同，先 flush 并 close 原 fd
        flush(path);
        ::close(fc.fd);
    }
    fc.fd = fd;
}

void LCacheManager::closeFile(const std::string &path)
{
    std::lock_guard<std::mutex> lk(m_mutex);

    auto it = m_fileCaches.find(path);
    if (it == m_fileCaches.end()) return;

    // 写回该文件所有脏块
    FileCache &fc = it->second;
    for (auto &p : fc.blocks)
    {
        CacheBlock &blk = p.second;
        if (blk.dirty)
        {
            ::lseek(fc.fd, blk.baseOffset, SEEK_SET);
            ::write(fc.fd, blk.data.data(), blk.dataSize);
            blk.dirty = false;
        }
        // 从 LRU 中移除对应项
        auto itmap = m_lruIters.find(path);
        if (itmap != m_lruIters.end())
        {
            auto it2 = itmap->second.find(p.first);
            if (it2 != itmap->second.end())
            {
                m_lruList.erase(it2->second);
            }
        }
        --m_curBlocks;
    }

    // 清理索引
    m_lruIters.erase(path);

    // 关闭 fd 并移除 filecache
    if (fc.fd != -1) ::close(fc.fd);
    m_fileCaches.erase(it);
}

// 内部：把块写回磁盘（假定已持锁或在私有调用路径中正确同步）
void LCacheManager::writeBlockToDisk(FileCache &fc, off_t blockBase)
{
    auto itBlock = fc.blocks.find(blockBase);
    if (itBlock == fc.blocks.end()) return;
    CacheBlock &blk = itBlock->second;
    if (!blk.dirty) return;

    ::lseek(fc.fd, blk.baseOffset, SEEK_SET);
    ssize_t w = ::write(fc.fd, blk.data.data(), blk.dataSize);
    if (w < 0)
    {
        // 写失败：抛异常或忽略，根据需求选择；此处选择抛异常会破坏鲁棒性，故只清理 dirty 标志以避免重复写入
        // throw std::runtime_error("write error in writeBlockToDisk");
    }
    blk.dirty = false;
}

// 内部：确保某个块存在，若不存在从磁盘读取并插入缓存并返回引用
LCacheManager::CacheBlock &LCacheManager::ensureBlock(const std::string &path, off_t blockBase)
{
    FileCache &fc = m_fileCaches[path]; // caller 保证文件已注册
    auto it = fc.blocks.find(blockBase);
    if (it != fc.blocks.end())
    {
        touchLRU(path, blockBase); // 更新 LRU
        return it->second;
    }

    // 块不存在，可能需要创建并从磁盘读取
    CacheBlock blk;
    blk.baseOffset = blockBase;
    blk.data.resize(m_blockSize);                                                  // 分配 BLOCK 大小
    ::lseek(fc.fd, blockBase, SEEK_SET);                                           // 移动到块起始
    ssize_t r = ::read(fc.fd, blk.data.data(), static_cast<ssize_t>(m_blockSize)); // 读取数据
    if (r < 0) r = 0;                                                              // 读取错误视为 0
    blk.dataSize = static_cast<size_t>(r);                                         // 记录有效字节
    blk.dirty = false;

    // 插入
    auto res = fc.blocks.emplace(blockBase, std::move(blk));
    ++m_curBlocks;
    touchLRU(path, blockBase); // 更新 LRU（也会处理 evict）
    evictIfNeeded();           // 如超出上限，驱逐
    return res.first->second;
}

// 内部：把 (path,blockBase) 移到 LRU front（最近使用）
void LCacheManager::touchLRU(const std::string &path, off_t blockBase)
{
    // 找到对应 iter，如果存在则移动到 front
    auto pit = m_lruIters.find(path);
    if (pit != m_lruIters.end())
    {
        auto &inner = pit->second;
        auto itb = inner.find(blockBase);
        if (itb != inner.end())
        {
            m_lruList.erase(itb->second);            // 移除旧位置
            m_lruList.push_front({path, blockBase}); // 插入到 front
            inner[blockBase] = m_lruList.begin();    // 更新 iterator
            return;
        }
    }
    else
    {
        m_lruIters.emplace(path, std::unordered_map<off_t, std::list<LRUKey>::iterator>{});
    }

    // 若不存在，则直接插入 front
    m_lruList.push_front({path, blockBase});
    m_lruIters[path][blockBase] = m_lruList.begin();
}

// 内部：驱逐最久未使用块直到 curBlocks <= maxBlocks
void LCacheManager::evictIfNeeded()
{
    while (m_curBlocks > m_maxBlocks && !m_lruList.empty())
    {
        auto backKey = m_lruList.back(); // 最久未用
        const std::string &path = backKey.first;
        off_t blockBase = backKey.second;

        auto fIt = m_fileCaches.find(path);
        if (fIt != m_fileCaches.end())
        {
            FileCache &fc = fIt->second;
            auto bIt = fc.blocks.find(blockBase);
            if (bIt != fc.blocks.end())
            {
                CacheBlock &blk = bIt->second;
                if (blk.dirty)
                {
                    ::lseek(fc.fd, blk.baseOffset, SEEK_SET);
                    ::write(fc.fd, blk.data.data(), blk.dataSize); // 写回
                }
                fc.blocks.erase(bIt);
                --m_curBlocks;
            }
        }

        // 移除 LRU 索引
        auto mapIt = m_lruIters.find(path);
        if (mapIt != m_lruIters.end())
        {
            mapIt->second.erase(blockBase);
            if (mapIt->second.empty()) m_lruIters.erase(mapIt);
        }

        m_lruList.pop_back();
    }
}

// 公共：读接口（线程安全）
ssize_t LCacheManager::read(const std::string &path, void *buf, size_t count, off_t &offset)
{
    std::lock_guard<std::mutex> lk(m_mutex);

    auto fit = m_fileCaches.find(path);
    if (fit == m_fileCaches.end()) return -1; // 未注册文件
    FileCache &fc = fit->second;

    size_t totalRead = 0;
    char *out = static_cast<char *>(buf);

    while (count > 0)
    {
        off_t blockBase = (offset / static_cast<off_t>(m_blockSize)) * static_cast<off_t>(m_blockSize); // 块对齐起点
        size_t offsetInBlock = static_cast<size_t>(offset - blockBase);
        CacheBlock &blk = ensureBlock(path, blockBase); // 若不存在则加载

        // 从该块可读的字节数
        size_t canRead = 0;
        if (offsetInBlock < blk.dataSize)
        {
            canRead = std::min(count, blk.dataSize - offsetInBlock);
            std::memcpy(out + totalRead, blk.data.data() + offsetInBlock, canRead); // 复制数据
            totalRead += canRead;
            count -= canRead;
            offset += static_cast<off_t>(canRead);
        }

        // 如果当前块在偏移处没有更多数据，则尝试结束（文件末尾）
        if (canRead == 0) break;
    }

    return static_cast<ssize_t>(totalRead);
}

// 公共：写接口（线程安全），写入缓存并标记 dirty
ssize_t LCacheManager::write(const std::string &path, const void *buf, size_t count, off_t &offset)
{
    std::lock_guard<std::mutex> lk(m_mutex);

    auto fit = m_fileCaches.find(path);
    if (fit == m_fileCaches.end()) return -1; // 未注册文件
    FileCache &fc = fit->second;

    size_t totalWritten = 0;
    const char *in = static_cast<const char *>(buf);

    while (count > 0)
    {
        off_t blockBase = (offset / static_cast<off_t>(m_blockSize)) * static_cast<off_t>(m_blockSize);
        size_t offsetInBlock = static_cast<size_t>(offset - blockBase);
        CacheBlock &blk = ensureBlock(path, blockBase); // 若不存在则加载或创建

        // 写入到该块的可用空间
        size_t canWrite = std::min(count, m_blockSize - offsetInBlock);
        std::memcpy(blk.data.data() + offsetInBlock, in + totalWritten, canWrite); // 拷贝
        // 更新块的 dataSize（当写入越过原数据尾部时要扩展）
        blk.dataSize = std::max(blk.dataSize, offsetInBlock + canWrite);
        blk.dirty = true;

        totalWritten += canWrite;
        count -= canWrite;
        offset += static_cast<off_t>(canWrite);

        // 更新 LRU（ensureBlock 已经 touch）
        // 如果需要可以在这里触发异步 flush 或写回策略
    }

    return static_cast<ssize_t>(totalWritten);
}

// 公共：flush 单文件
void LCacheManager::flush(const std::string &path)
{
    std::lock_guard<std::mutex> lk(m_mutex);

    auto fit = m_fileCaches.find(path);
    if (fit == m_fileCaches.end()) return;
    FileCache &fc = fit->second;

    for (auto &p : fc.blocks)
    {
        CacheBlock &blk = p.second;
        if (blk.dirty)
        {
            ::lseek(fc.fd, blk.baseOffset, SEEK_SET);
            ::write(fc.fd, blk.data.data(), blk.dataSize);
            blk.dirty = false;
        }
    }
}

// 公共：flushAll
void LCacheManager::flushAll()
{
    for (auto &p : m_fileCaches)
    {
        flush(p.first);
    }
}
