/**
 * @file lcachemanager.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 全局文件缓存管理器源文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 */

#include "lcachemanager.h"

#include <cstring>
#include <stdexcept>
#include <algorithm>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


LCacheManager::LCacheManager() : m_blockSize(4096), m_maxBlocks(1024), m_curBlocks(0)
{
    // 默认 BLOCK = 4KiB, 最大 1024 块，总计 4MiB 缓存（可通过 setMaxBlocks 调整）。
}

LCacheManager::~LCacheManager()
{
    // 析构时写回所有缓存，并注意线程保护。
    std::lock_guard<std::mutex> lk(m_mutex);

    flushAll();
}

LCacheManager &LCacheManager::instance()
{
    static LCacheManager s_instance;


    return s_instance;
}

void LCacheManager::setMaxBlocks(size_t maxBlocks)
{
    std::lock_guard<std::mutex> lk(m_mutex);

    m_maxBlocks = maxBlocks;
    // 驱逐超出限制的块。
    evictIfNeeded();
}

void LCacheManager::addFile(const std::string &filePath, int fd)
{
    std::lock_guard<std::mutex> lk(m_mutex);

    // 若不存在则创建。
    FileCache &fc = m_fileCaches[filePath];

    if (-1 != fc.fd && fc.fd != fd)
    {
        // 如果原来已经有 fd 并且不同，先 flush 并 close 原 fd。
        flush(filePath);
        ::close(fc.fd);
    }
    fc.fd = fd;
}

void LCacheManager::closeFile(const std::string &filePath)
{
    std::lock_guard<std::mutex> lk(m_mutex);

    auto it = m_fileCaches.find(filePath);
    if (m_fileCaches.end() == it) return;

    // 写回该文件所有脏块。
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

        // 从 LRU 中移除对应项。
        auto itmap = m_lruIters.find(filePath);
        if (m_lruIters.end() != itmap)
        {
            auto it2 = itmap->second.find(p.first);
            if (it2 != itmap->second.end()) m_lruList.erase(it2->second);
        }

        --m_curBlocks;
    }

    // 清理索引。
    m_lruIters.erase(filePath);

    // 关闭 fd 并移除 filecache。
    if (-1 != fc.fd) ::close(fc.fd);
    m_fileCaches.erase(it);
}

void LCacheManager::writeBlockToDisk(FileCache &fc, off_t blockBase)
{
    auto itBlock = fc.blocks.find(blockBase);
    if (itBlock == fc.blocks.end()) return;

    CacheBlock &blk = itBlock->second;
    if (!blk.dirty) return;

    ::lseek(fc.fd, blk.baseOffset, SEEK_SET);

    ssize_t w = ::write(fc.fd, blk.data.data(), blk.dataSize);
    if (w < 0) throw std::runtime_error("write error in writeBlockToDisk");

    blk.dirty = false;
}

LCacheManager::CacheBlock &LCacheManager::ensureBlock(const std::string &filePath, off_t blockBase)
{
    // caller 保证文件已注册。
    FileCache &fc = m_fileCaches[filePath];

    auto it = fc.blocks.find(blockBase);
    if (it != fc.blocks.end())
    {
        // 更新 LRU。
        updateLRU(filePath, blockBase);
        return it->second;
    }

    // 块不存在，可能需要创建并从磁盘读取。
    CacheBlock blk;
    blk.baseOffset = blockBase;
    // 分配 BLOCK 大小。
    blk.data.resize(m_blockSize);

    // 移动到块起始。
    ::lseek(fc.fd, blockBase, SEEK_SET);

    // 读取数据。
    ssize_t r = ::read(fc.fd, blk.data.data(), static_cast<ssize_t>(m_blockSize));
    // 读取错误视为 0。
    if (r < 0) r = 0;

    // 记录有效字节。
    blk.dataSize = static_cast<size_t>(r);
    blk.dirty = false;

    // 插入
    auto res = fc.blocks.emplace(blockBase, std::move(blk));
    ++m_curBlocks;

    // 更新 LRU（也会处理淘汰 evict 的情况）。
    updateLRU(filePath, blockBase);
    // 如超出上限，驱逐。
    evictIfNeeded();


    return res.first->second;
}

void LCacheManager::updateLRU(const std::string &filePath, off_t blockBase)
{
    // 找到对应 iter，如果存在则移动到 front。
    auto pit = m_lruIters.find(filePath);
    if (m_lruIters.end() != pit)
    {
        auto &inner = pit->second;
        auto itb = inner.find(blockBase);
        if (inner.end() != itb)
        {
            // 移除旧位置。
            m_lruList.erase(itb->second);
            // 插入到 front。
            m_lruList.push_front({filePath, blockBase});
            // 更新 iterator。
            inner[blockBase] = m_lruList.begin();


            return;
        }
    }
    else
    {
        m_lruIters.emplace(filePath, std::unordered_map<off_t, std::list<LRUKey>::iterator>{});
    }

    // 若不存在，则直接插入 front。
    m_lruList.push_front({filePath, blockBase});
    m_lruIters[filePath][blockBase] = m_lruList.begin();
}

void LCacheManager::evictIfNeeded()
{
    while (m_curBlocks > m_maxBlocks && !m_lruList.empty())
    {
        // 最久未用。
        auto backKey = m_lruList.back();
        const std::string &filePath = backKey.first;
        off_t blockBase = backKey.second;

        auto fIt = m_fileCaches.find(filePath);
        if (m_fileCaches.end() != fIt)
        {
            FileCache &fc = fIt->second;
            auto bIt = fc.blocks.find(blockBase);
            if (fc.blocks.end() != bIt)
            {
                CacheBlock &blk = bIt->second;
                if (blk.dirty)
                {
                    ::lseek(fc.fd, blk.baseOffset, SEEK_SET);
                    // 写回。
                    ::write(fc.fd, blk.data.data(), blk.dataSize);
                }
                fc.blocks.erase(bIt);
                --m_curBlocks;
            }
        }

        // 移除 LRU 索引。
        auto mapIt = m_lruIters.find(filePath);
        if (mapIt != m_lruIters.end())
        {
            mapIt->second.erase(blockBase);
            if (mapIt->second.empty()) m_lruIters.erase(mapIt);
        }

        m_lruList.pop_back();
    }
}

ssize_t LCacheManager::read(const std::string &filePath, void *buf, size_t count, off_t &offset)
{
    std::lock_guard<std::mutex> lk(m_mutex);

    auto fit = m_fileCaches.find(filePath);
    // 未注册文件。
    if (m_fileCaches.end() == fit) return -1;

    FileCache &fc = fit->second;
    size_t totalRead = 0;
    char *out = static_cast<char *>(buf);

    while (count > 0)
    {
        // 块对齐起点。
        off_t blockBase = (offset / static_cast<off_t>(m_blockSize)) * static_cast<off_t>(m_blockSize);
        size_t offsetInBlock = static_cast<size_t>(offset - blockBase);
        // 若不存在则加载。
        CacheBlock &blk = ensureBlock(filePath, blockBase);

        // 从该块可读的字节数。
        size_t canRead = 0;
        if (offsetInBlock < blk.dataSize)
        {
            canRead = std::min(count, blk.dataSize - offsetInBlock);
            // 复制数据。
            std::memcpy(out + totalRead, blk.data.data() + offsetInBlock, canRead);
            totalRead += canRead;
            count -= canRead;
            offset += static_cast<off_t>(canRead);
        }

        // 如果当前块在偏移处没有更多数据，则尝试结束（文件末尾）。
        if (0 == canRead) break;
    }


    return static_cast<ssize_t>(totalRead);
}

ssize_t LCacheManager::write(const std::string &filePath, const void *buf, size_t count, off_t &offset)
{
    std::lock_guard<std::mutex> lk(m_mutex);

    auto fit = m_fileCaches.find(filePath);
    // 未注册文件。
    if (fit == m_fileCaches.end()) return -1;

    FileCache &fc = fit->second;
    size_t totalWritten = 0;
    const char *in = static_cast<const char *>(buf);

    while (count > 0)
    {
        off_t blockBase = (offset / static_cast<off_t>(m_blockSize)) * static_cast<off_t>(m_blockSize);
        size_t offsetInBlock = static_cast<size_t>(offset - blockBase);
        // 若不存在则加载或创建。
        CacheBlock &blk = ensureBlock(filePath, blockBase);

        // 写入到该块的可用空间。
        size_t canWrite = std::min(count, m_blockSize - offsetInBlock);
        // 拷贝。
        std::memcpy(blk.data.data() + offsetInBlock, in + totalWritten, canWrite);
        // 更新块的 dataSize（当写入越过原数据尾部时要扩展）。
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

void LCacheManager::flush(const std::string &filePath)
{
    std::lock_guard<std::mutex> lk(m_mutex);

    auto fit = m_fileCaches.find(filePath);
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

void LCacheManager::flushAll()
{
    for (auto &p : m_fileCaches)
    {
        flush(p.first);
    }
}
