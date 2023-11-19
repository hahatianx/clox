
#include "common.h"

#include "value/value.h"
#include "value/object.h"
#include "value/object/function.h"

object_function_t* new_function() {
    object_function_t* function = ALLOCATE_OBJECT(object_function_t, OBJ_FUNCTION);
    function->arity = 0;
    function->name = NULL;
    init_chunk(&function->chunk);
    return function;
}