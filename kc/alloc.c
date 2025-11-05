#include <stdlib.h>
#include <stdio.h>

#include "../include/alloc.h"

void* Alloc(uint64_t size) {
    if (!size) {
        puts("Alloc(): size = 0");
        abort();
    }

    void* block = malloc(size);
    if (!block) {
        puts("Alloc(): Out of memory");
        abort();
    }

    return block;
}

void* Resize(void* block, uint64_t size) {
    if (!block) {
        puts("Resize(): block == NULL");
        abort();
    }

    if (!size) {
        puts("Resize(): size = 0");
        abort();
    }

    void* ret = realloc(block, size);
    if (!ret) {
        puts("Resize(): Out of memory");
        abort();
    }

    return ret;
}

void Free(void* block) {
    if (!block) {
        puts("Free(): block == NULL");
        abort();
    }

    free(block);
}