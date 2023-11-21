#ifndef CLOX_MEMORY_H_
#define CLOX_MEMORY_H_

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "error/error.h"


#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) << 1)

#define GROW_ARRAY(type, pointer, old_count, new_count) \
    (type*)reallocate(pointer, sizeof(type) * (old_count), \
        sizeof(type) * (new_count))

#define FREE_ARRAY(type, pointer, old_count) \
    reallocate(pointer, sizeof(type) * (old_count), 0);

#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define FREE(type, pointer) \
    reallocate(pointer, sizeof(type), 0)

#define alloc_block(type, return_ptr) \
    do {  \
        type* ptr = (type*)malloc(sizeof(type)); \
        if (ptr == NULL) {                       \
            __CLOX_ERROR("Could not allocate enough memory."); \
        }                                        \
        *(return_ptr) = ptr;                       \
    } while (0)

#define alloc_struct(type, member, return_ptr) \
    do {  \
        type* ptr = (type*)malloc(sizeof(type)); \
        if (ptr == NULL) {                       \
            __CLOX_ERROR("Could not allocate enough memory."); \
        }                                        \
        *(return_ptr) = &((type*)ptr)->member;                       \
    } while (0)

__attribute__((unused)) void* reallocate(void* pointer, size_t old_count, size_t new_size);

#endif