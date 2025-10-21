#include <iostream>
#include <thread>

#if __unix__

#include <unistd.h>

#endif


void doIt()
{
    std::cout << "thread pid: " << getpid() << std::endl;
}


int main()
{
    std::cout << "parent pid: " << getpid() << std::endl;

    std::thread t(doIt);

    t.join();


    return 0;
}
