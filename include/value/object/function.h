#ifndef __CLOX_OBJECT_FUNCTION_H__
#define __CLOX_OBJECT_FUNCTION_H__

#include "basic/chunk.h"

#include "value/value.h"
#include "value/object.h"
#include "value/object/string.h"

typedef struct clox_function object_function_t;

struct clox_function {
    struct clox_object obj;
    int arity;
    chunk_t chunk;
    object_string_t* name;
};

object_function_t* new_function();

#define IS_FUNCTION(value) is_object_type(value, OBJ_FUNCTION);

#define AS_FUNCTION(value) ((object_function_t*)AS_OBJECT(value))

#endif