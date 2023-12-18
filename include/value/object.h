#ifndef CLOX_OBJECT_H_
#define CLOX_OBJECT_H_

#include "common.h"

#include "utils/linklist.h"
#include "basic/memory.h"
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

extern list_t temporary_objs;

static inline bool is_object_type(value_t value, object_type_t type) {
    return IS_OBJECT(value) && (AS_OBJECT(value))->type == type;
}

object_t* allocate_object(size_t size, object_type_t type);
#define ALLOCATE_OBJECT(type, object_type) \
    (type*)allocate_object(sizeof(type), object_type)


int print_object(value_t value);
void blacken_object(object_t* obj);
void merge_temporary();

void free_object(object_t* obj);

#endif