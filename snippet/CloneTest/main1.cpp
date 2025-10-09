// 使用包裹函数 clone() 创建进程。
#include <iostream>
#include <exception>
#include <cstring>

#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <sys/wait.h>


#define ERR_EXIT(func)      \
    do                      \
    {                       \
        perror(func);       \
        exit(EXIT_FAILURE); \
    } while (0)


// 这个栈的大小比较合适，小了的话容易爆栈。
#define STACK_SIZE 1024 * 1024


int doIt(void *args)
{
    std::cout << "child pid: " << getpid() << std::endl;

    char *pMsg = (char *)args;

    strcpy(pMsg, "created a thread.");

    // 模拟子进程执行一些工作。
    sleep(2);


    return 0;
}


int main()
{
    std::cout << "parent pid: " << getpid() << std::endl;

    char *stack = new char[STACK_SIZE];
    if (!stack) throw std::exception();

    char msg[100] = "created a process.";

    std::cout << "before: " << msg << std::endl;

    // stack 应指向栈顶，即起始地址加上栈大小。
    // clone()：返回线程 ID（进程即为线程组的主线程 ID），即 tid。
    int tid = clone(doIt, stack + STACK_SIZE, SIGCHLD, msg);

    if (-1 == tid) ERR_EXIT("clone");

    int res = waitpid(-1, nullptr, 0);
    if (-1 == res) ERR_EXIT("waitpid");

    std::cout << "after: " << msg << std::endl;

    delete[] stack;
    stack = nullptr;


    return 0;
}
