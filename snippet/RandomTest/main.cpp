#include <iostream>

#include "lrandom.h"


int main()
{
    for (int i = 0; i < 5; ++i) std::cout << LRandom::genRandomNumber(1, 100) << ' ';
    std::cout << std::endl;

    std::cout << std::endl;

    std::vector<int> vec = LRandom::genRandomVector(1, 100, 10000);
    for (int i = 0; i < vec.size(); ++i) std::cout << vec[i] << ' ';
    std::cout << std::endl;


    return 0;
}
