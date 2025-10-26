/**
 * @file lthreadpool.cpp
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 线程池类源文件。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#include "lthreadpool.h"


LThreadPool::LThreadPool(size_t threads) : stop(false)
{
    // 初始化 stop 标志为 false，并启动 threads 个工作线程。
    // 每个线程不断轮询等待任务：
    // 1. 锁住任务队列 mutex；
    // 2. 使用 condition_variable 等待新任务或 stop 信号；
    // 3. 若 stop 且任务队列为空，则退出线程循环；
    // 4. 否则取出队列首任务并解锁 mutex；
    // 5. 执行任务。
    for (size_t i = 0; i < threads; ++i)
        workers.emplace_back(
            [this]
            {
                for (;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);

                        // 等待任务到来或线程池停止。
                        this->condition.wait(lock, [this]
                                             { return this->stop || !this->tasks.empty(); });

                        // 如果线程池停止且任务队列为空，退出线程。
                        if (this->stop && this->tasks.empty()) return;

                        // 取出队列首任务并从队列移除。
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    // 执行任务（解锁后执行）。
                    task();
                }
            });
}

LThreadPool::~LThreadPool()
{
    {
        // 析构时设置 stop 标志，通知所有线程退出等待。然后 join 每个工作线程，保证线程安全退出。
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;

        // 唤醒所有等待线程，确保它们能检查 stop 并退出。
        condition.notify_all();
    }

    // 等待所有工作线程退出。
    for (std::thread &worker : workers) worker.join();
}
