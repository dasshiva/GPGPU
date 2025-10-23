#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <stdint.h>

void* Alloc(uint64_t size);
void* Resize(void* block, uint64_t size);
void  Free(void* block);

#endif