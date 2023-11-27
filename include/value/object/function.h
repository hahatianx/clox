#ifndef CLOX_OBJECT_FUNCTION_H_
#define CLOX_OBJECT_FUNCTION_H_

#include "basic/chunk.h"

#include "value/value.h"
#include "value/object.h"
#include "value/object/string.h"
#include "constant.h"

typedef value_t (* native_fn_t)(int argc, value_t* args);

typedef struct clox_native_func object_native_func_t;

struct clox_native_func {
    struct clox_object obj;
    int arity;
    native_fn_t function;
};

object_native_func_t* new_native(int argc, native_fn_t func);

typedef struct clox_upvalue object_upvalue_t;

struct clox_upvalue {
    object_t obj;
    list_link_t link;

    value_t closed;
    value_t* location;
    bool mutable;
};

typedef struct clox_function object_function_t;

typedef struct {
    uint8_t index;
    bool is_local;
} upvalue_t;

object_upvalue_t* new_upvalue(value_t* slot);

struct clox_function {
    struct clox_object obj;
    int              arity;

    upvalue_t        upvalues[UINT8_COUNT];
    int              upvalue_count;

    chunk_t          chunk;
    object_string_t* name;
};

object_function_t* new_function();

typedef struct clox_closure object_closure_t;

struct clox_closure {
    struct clox_object obj;
    object_function_t *function;

    object_upvalue_t**  upvalues;
    int upvalue_count;
};

object_closure_t* new_closure(object_function_t* function);

#define IS_UPVALUE(value)  is_object_type(value, OBJ_UPVALUE)
#define AS_UPVALUE(value)  ((object_upvalue_t*)AS_OBJECT(value))

#define IS_FUNCTION(value) is_object_type(value, OBJ_FUNCTION)
#define AS_FUNCTION(value) ((object_function_t*)AS_OBJECT(value))

#define IS_NATIVE(value)   is_object_type(value, OBJ_NATIVE)
#define AS_NATIVE(value)   (((object_native_func_t *)AS_OBJECT(value))->function)

#define IS_CLOSURE(value)  is_object_type(value, OBJ_CLOSURE)
#define AS_CLOSURE(value)  ((object_closure_t *)AS_OBJECT(value))

#endif