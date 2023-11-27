#include "common.h"

#include "vm/runtime.h"

object_function_t* new_function() {
    object_function_t* function = ALLOCATE_OBJECT(object_function_t, OBJ_FUNCTION);
    function->arity = 0;
    function->upvalue_count = 0;
    function->name = NULL;
    init_chunk(&function->chunk);
    return function;
}

object_native_func_t* new_native(int argc, native_fn_t func) {
    object_native_func_t* native = ALLOCATE_OBJECT(object_native_func_t, OBJ_NATIVE);
    native->arity = argc;
    native->function = func;
    return native;
}

object_closure_t* new_closure(object_function_t* function) {
    object_upvalue_t** upvalues = ALLOCATE(object_upvalue_t*, function->upvalue_count);
    for (int i = 0; i < function->upvalue_count; ++i) {
        upvalues[i] = NULL;
    }

    object_closure_t* closure = ALLOCATE_OBJECT(object_closure_t, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalue_count = function->upvalue_count;
    function->upvalue_count = 0;
    return closure;
}

object_upvalue_t* new_upvalue(value_t* slot) {
    object_upvalue_t* upvalue = ALLOCATE_OBJECT(object_upvalue_t, OBJ_UPVALUE);
    upvalue->location = slot;
    upvalue->closed = NIL_VAL;
    list_link_init(&upvalue->link);
    return upvalue;
}