#ifndef CLOX_MEMORY_H_
#define CLOX_MEMORY_H_

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "error/error.h"
#include "value/value.h"


/*
 * NOTE: ALLOCATE vs realloc macros
 *    - ALLOCATE macro does not trigger garbage collection mechanism.
 *    - All other realloc macros trigger garbage collection mechanisms
 *
 * WARN: common errors when calling multiple realloc macros simultaneously
 *    - might trigger garbage collection and clean up objects that are not fully mounted to the VM
 *      e.g.  {
 *                allocate_object { gc, allocation, add object to vm obj list}
 *                allocate        { gc again, allocation, .... }
 *            }
 *          In this example, since the object is just added to the obj list, but not attached to either
 *             vm stacks, var tables, upvalues, closures, functions, the object allocated would be wiped
 *             out in the second gc sweeper.
 */

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) << 1)

#define GROW_ARRAY(type, pointer, old_count, new_count) \
    (type*)reallocate(pointer, sizeof(type) * (old_count), \
        sizeof(type) * (new_count))

#define FREE_ARRAY(type, pointer, old_count) \
    reallocate(pointer, sizeof(type) * (old_count), 0);

#define ALLOCATE(type, count) \
    (type*)malloc(sizeof(type) * (count))

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

void mark_object(object_t* object);
void mark_value(value_t value);
void mark_array(value_array_t* array);
__attribute__((unused)) void* reallocate(void* pointer, size_t old_count, size_t new_size);
void collect_garbage();

#endif