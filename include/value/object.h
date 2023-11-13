#ifndef __CLOX_OBJECT_H__
#define __CLOX_OBJECT_H__

#include "common.h"

#include "utils/linklist.h"
#include "basic/memory.h"
#include "vm/vm.h"
#include "value/value.h"

/**
 * clox_object is defined in value/value.h
 * 

typedef enum {
    OBJ_STRING,
} object_type_t;

typedef struct clox_object {
    object_type_t type;
    list_link_t link;
} object_t;
**/

#define OBJ_TYPE(value)    (AS_OBJECT(value)->type)

static inline bool is_object_type(value_t value, object_type_t type) {
    return IS_OBJECT(value) && (AS_OBJECT(value))->type == type;
}

static object_t* allocate_object(size_t size, object_type_t type) {
    object_t* object = (object_t*)reallocate(NULL, 0, size);
    object->type = type;
    list_link_init(&object->link);
    // list_insert_head(&vm.obj, &object->link);
    return object;
}

#define ALLOCATE_OBJECT(type, object_type) \
    (type*)allocate_object(sizeof(type), object_type)


#ifdef DEBUG_PRINT_CODE
int print_object(value_t value);
#else
void print_object(value_t value);
#endif

void free_object(object_t* obj);

#endif