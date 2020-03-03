#include "alloctor.h"
#include "memory_mg.hpp"


void* operator new(size_t size)
{
	return MemoryMg::Instance().alloc_mem(size);
}

void operator delete(void *p)
{
	MemoryMg::Instance().free_mem(p);
}

void* operator new[](size_t size)
{
	return MemoryMg::Instance().alloc_mem(size);
}

void operator delete[](void* p)
{
	MemoryMg::Instance().free_mem(p);
}

void* mem_alloc(size_t size)
{
	return malloc(size);
}
void mem_free(void* p)
{
	free(p);
}