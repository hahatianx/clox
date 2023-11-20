#include "common.h"

#include "value/object/function.h"

object_function_t* new_function() {
    object_function_t* function = ALLOCATE_OBJECT(object_function_t, OBJ_FUNCTION);
    function->arity = 0;
    function->name = NULL;
    init_chunk(&function->chunk);
    return function;
}

object_native_func_t* new_native(native_fn_t func) {
    object_native_func_t* native = ALLOCATE_OBJECT(object_native_func_t, OBJ_NATIVE);
    native->function = func;
    return native;
}