
#include "MemoryControl.h"
#include <stdlib.h>

//#include <iostream>
//using namespace std;

void* operator new(size_t size)
{
	return MemoryManger::getInstance().allocMemory(size);
}

void operator delete(void* p) noexcept
{
	MemoryManger::getInstance().deAllocMemory(p);
}

void* operator new[](size_t size)
{
	return MemoryManger::getInstance().allocMemory(size);
}

void operator delete[](void* p) noexcept
{
	MemoryManger::getInstance().deAllocMemory(p);
}
