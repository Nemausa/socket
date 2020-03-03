#ifndef ALLOCTOR_H
#define ALLOCTOR_H

/**
* @file alloctor.h
*
* @brief This message displayed in Doxygen Files index
*
* @ingroup PackageName
* (note: this needs exactly one @defgroup somewhere)
*
* @date	2020-03-02
* @author morris
* contact: tappanmorris@outlook.com
*
*/

void* operator new(size_t size);
void operator delete(void *p);
void* operator new[](size_t size);
void  operator delete[](void* p);
void* mem_alloc(size_t size);
void mem_free(void* p);


#endif  // ALLOCTOR_H