// 使用包裹函数 clone() 创建线程。
// 具体注释见 snippet/FutexTest/
#include <iostream>
#include <exception>
#include <cstring>

#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <sys/wait.h>

#include "lfutex.h"


#define ERR_EXIT(func)      \
    do                      \
    {                       \
        perror(func);       \
        exit(EXIT_FAILURE); \
    } while (0)


#define STACK_SIZE 1024 * 1024


static int futexVal = 0;


int doIt(void *args)
{
    std::cout << "child thread: child thread is working..." << std::endl;

    std::cout << "child thread: pid: " << getpid() << std::endl;
    std::cout << "child thread: tid: " << gettid() << std::endl;

    char *pMsg = (char *)args;

    strcpy(pMsg, "created a thread.");

    // 模拟子线程执行一些工作。
    sleep(2);

    futexVal = -1;
    LFutex::futexWake(&futexVal, 1);


    return 0;
}


int main()
{
    std::cout << "main thread: pid: " << getpid() << std::endl;
    std::cout << "main thread: tid: " << gettid() << std::endl;

    char *stack = new char[STACK_SIZE];
    if (!stack) throw std::exception();

    char msg[100] = "created a process.";

    std::cout << "main thread: msg before: \"" << msg << "\"" << std::endl;

    int tid = clone(doIt, stack + STACK_SIZE, SIGCHLD | CLONE_VM | CLONE_THREAD | CLONE_SIGHAND, msg);
    if (-1 == tid) ERR_EXIT("clone");

    std::cout << "main thread: waiting for child thread to finish..." << std::endl;

    // 等待子线程结束
    while (0 == futexVal) LFutex::futexWait(&futexVal, 0);

    std::cout << "main thread: child thread has finished." << std::endl;

    std::cout << "main thread: msg after: \"" << msg << "\"" << std::endl;

    // 回收子线程资源
    delete[] stack;
    stack = nullptr;


    return 0;
}
