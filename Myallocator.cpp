#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include "Myallocator.h"

int main()
{
    /*CMyallocator<int, 4, 10> alloc;
    int* p = alloc.allocate(1);
    alloc.deallocate(p, 1);*/
    std::vector<int, CMyallocator<int, 65536, 2>> test_my_alloc;

    clock_t start_my = clock();

    for(int i = 0; i<10000; ++i)
    {
        test_my_alloc.push_back(i);
    }

    clock_t end_my = clock();
    std::cout << (double)(end_my - start_my)/CLOCKS_PER_SEC << std::endl;

    std::vector<int, std::allocator<int>> test_std_alloc;

    clock_t start_std = clock();

    for(int i = 0; i<10000; ++i)
    {
        test_std_alloc.push_back(i);
    }

    clock_t end_std = clock();

    std::cout << (double)(end_std - start_std)/CLOCKS_PER_SEC << std::endl; 
}