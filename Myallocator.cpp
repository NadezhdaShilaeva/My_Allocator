#include <iostream>
#include <vector>
#include "Myallocator.h"

int main()
{
    /*CMyallocator<int, 4, 10> alloc;
    int* p = alloc.allocate(1);
    alloc.deallocate(p, 1);*/
    std::vector<int, CMyallocator<int, 4, 10, 40, 1>> test_vec(10);
    std::cout << "Hello World!\n";
}