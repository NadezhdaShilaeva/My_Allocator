#include <iostream>
#include <vector>
#include <ctime>
#include "Myallocator.h"

int main()
{
    clock_t start_std = clock();

    std::vector<int, std::allocator<int>> test_std_alloc(10);

    for (int i = 0; i < 200000; ++i)
    {
        test_std_alloc.push_back(i);
    }
    while (!test_std_alloc.empty())
    {
        test_std_alloc.erase(test_std_alloc.begin());
    }

    clock_t end_std = clock();
    std::cout << (double)(end_std - start_std) / CLOCKS_PER_SEC << std::endl;


    clock_t start_my = clock();

    std::vector<int, CMyallocator<int, 1310720, 2>> test_my_alloc(10);

    for (int i = 0; i < 200000; ++i)
    {
        test_my_alloc.push_back(i);
    }
    while (!test_my_alloc.empty())
    {
        test_my_alloc.erase(test_my_alloc.begin());
    }

    clock_t end_my = clock();
    std::cout << (double)(end_my - start_my) / CLOCKS_PER_SEC << std::endl;
}