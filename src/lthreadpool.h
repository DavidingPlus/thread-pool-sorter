/**
 * @file lthreadpool.h
 * @author DavidingPlus (davidingplus@qq.com)
 * @brief 线程池类头文件。
 * @note 本线程池参考实现：https://github.com/progschj/ThreadPool
 * @details LThreadPool 是一个固定大小的线程池，实现了任务队列和线程管理。支持将任意可调用对象异步提交到线程池，返回 std::future 获取结果。内部通过 std::mutex 和 std::condition_variable 管理任务队列同步。当线程池销毁时，所有线程会安全退出。
 *
 * Copyright (c) 2025 电子科技大学 刘治学
 *
 */

#ifndef _LTHREADPOOL_H_
#define _LTHREADPOOL_H_

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>


/**
 * @class LThreadPool
 * @brief 一个简单的固定线程数线程池。
 *
 * @note 使用方法
 *   LThreadPool pool(num_threads);
 *   auto f = pool.enqueue(func, args...);
 *   result = f.get();
 */
class LThreadPool
{

public:

    /**
     * @brief 构造函数，创建指定数量的工作线程。
     * @param threads 工作线程数量。
     * @note 构造时启动所有线程，并等待任务队列中的任务执行。
     */
    LThreadPool(size_t threads);

    /**
     * @brief 析构函数。
     * @note 先设置 stop 标志，通知所有线程退出，然后 join 所有线程。
     */
    virtual ~LThreadPool();

    /**
     * @brief 向线程池提交一个可调用对象（函数、lambda 等）。
     * @tparam F 可调用对象类型。
     * @tparam Args 参数包类型。
     * @param f 可调用对象。。
     * @param args 可调用对象的参数。
     * @return std::future<return_type> 返回未来对象，可获取函数返回值。
     * @note 如果线程池已停止，将抛出 std::runtime_error。同时使用 std::packaged_task 封装任务，保证返回值可通过 std::future 获取。
     */
    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>;


private:

    /**
     * @brief 线程池中所有工作线程。
     */
    std::vector<std::thread> workers;

    /**
     * @brief 任务队列，存放待执行的函数对象。
     */
    std::queue<std::function<void()>> tasks;

    /**
     * @brief 队列同步互斥锁。
     */
    std::mutex queue_mutex;

    /**
     * @brief 条件变量，通知线程有新任务。
     */
    std::condition_variable condition;

    /**
     * @brief 停止标志，析构时设置，阻止新任务加入。
     */
    bool stop;
};


template <class F, class... Args>
inline auto LThreadPool::enqueue(F &&f, Args &&...args)
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    // 将函数和参数绑定到一个 packaged_task。
    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    // 获取 future 用于返回调用结果。
    std::future<return_type> res = task->get_future();

    std::unique_lock<std::mutex> lock(queue_mutex);

    // 不允许在线程池停止后加入任务。
    if (stop) throw std::runtime_error("enqueue on stopped LThreadPool");

    // 将任务封装成 void() 类型，放入队列。
    tasks.emplace([task]()
                  { (*task)(); });

    // 通知一个等待线程有新任务。
    condition.notify_one();


    return res;
}


#endif
