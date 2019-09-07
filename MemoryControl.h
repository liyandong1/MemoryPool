
#ifndef _MEMORYCONTROL_H_
#define _MEMORYCONTROL_H_

#include "MemoryManger.h"
//通过内存管理单元分配内存

void* operator new(size_t size);
void operator delete(void* p) noexcept;

void* operator new[](size_t size);
void operator delete[](void* p) noexcept;

#endif
