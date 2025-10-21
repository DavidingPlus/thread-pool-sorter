#include <iostream>

#include "lrandom.h"


int main()
{
    for (int i = 0; i < 5; ++i) std::cout << LRandom::genRandomNumber(1, 100) << ' ' << std::flush;

    std::cout << std::endl;


    return 0;
}
