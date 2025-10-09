#include <iostream>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>


static int futexVal = 0;


int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout) { return syscall(SYS_futex, uaddr, futex_op, val, timeout, nullptr, 0); }

void futexWait(int *uaddr, int val)
{
    // FUTEX_WAIT：检测 *uaddr 是否等于 val。若是，则休眠，等待 FUTEX_WAKE 操作。
    futex(uaddr, FUTEX_WAIT, val, nullptr);
}

void futexWake(int *uaddr, int val)
{
    // FUTEX_WAKE：唤醒 val 个在 futex 等待的执行体。val = INT_MAX，表示唤醒所有。
    futex(uaddr, FUTEX_WAKE, val, nullptr);
}

void *doIt(void *arg)
{
    std::cout << "child thread: child thread is working..." << std::endl;

    // 模拟子线程执行一些工作。
    sleep(2);

    futexVal = -1;
    futexWake(&futexVal, 1);

    // TODO 这里后面最好不要在执行任何操作了，从语义上讲主线程在这里已经被唤醒了，代表子线程应该是结束的状态。在这里继续操作，例如打印信息可能会出现问题。我遇到的某个问题就是信息打印了两次。目前尚不知道为什么。

    return nullptr;
}


int main()
{
    pthread_t thread;
    pthread_create(&thread, nullptr, doIt, nullptr);

    std::cout << "main thread: waiting for child thread to finish..." << std::endl;

    // 注意：FUTEX_WAKE 操作唤醒执行体以后，futexWait() 函数就返回和结束了，但是他返回时并不会帮我们检测 *uaddr 是否和 val 不同，也就是说即使 *uaddr 和 val 相等，也会被 FUTEX_WAKE 操作唤醒而返回。因此最好手动检查 futexVal 的值，不等则证明该唤醒，等于则重新调用而阻塞，因为有可能是被其他操作 FUTEX_WAKE 的。
    while (0 == futexVal) futexWait(&futexVal, 0);

    std::cout << "main thread: child thread has finished." << std::endl;

    pthread_join(thread, nullptr);


    return 0;
}
