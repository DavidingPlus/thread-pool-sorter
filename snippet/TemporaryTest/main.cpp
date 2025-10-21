#include <iostream>
#include <thread>


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
