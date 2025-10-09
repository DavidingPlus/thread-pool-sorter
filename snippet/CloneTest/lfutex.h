#ifndef _LFUTEX_H_
#define _LFUTEX_H_

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <linux/futex.h>


// TODO 个人认为 uaddr 无论是使用全局变量，还是共享内存，还是匿名内存映射，都不应该由用户直接管理，而应该由 LFutex 类管理，后续设计。

namespace
{
    int futex(int *uaddr, int futexOp, int val, const struct timespec *timeout) { return syscall(SYS_futex, uaddr, futexOp, val, timeout, nullptr, 0); }
}

namespace LFutex
{
    void futexWait(int *uaddr, int val) { futex(uaddr, FUTEX_WAIT, val, nullptr); }

    void futexWake(int *uaddr, int val) { futex(uaddr, FUTEX_WAKE, val, nullptr); }
}


#endif
