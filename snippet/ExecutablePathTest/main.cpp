#include <iostream>

#include "lutil.h"


int main()
{
    std::cout << "executableFullPath: " << LUtil::executableFullPath() << std::endl;
    std::cout << "executableDirectory: " << LUtil::executableDirectory() << std::endl;


    return 0;
}
