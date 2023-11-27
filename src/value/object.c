#include <stdio.h>

#include "basic/memory.h"

#include "value/object.h"
#include "value/object/string.h"
#include "value/object/function.h"

static int print_function(object_function_t* func) {
    if (!func->name) {
        return printf("<script>");
    }
    return printf("<fn %s>", func->name->chars);
}

int print_object(value_t value) {
    switch(OBJ_TYPE(value)) {
        case OBJ_STRING:
            return printf("%s", AS_CSTRING(value));
        case OBJ_FUNCTION:
            return print_function(AS_FUNCTION(value));
        case OBJ_NATIVE:
            return printf("<native fn>");
        case OBJ_CLOSURE:
            return print_function(AS_CLOSURE(value)->function);
        case OBJ_UPVALUE:
            return printf("[upvalue]");
    }
    return 0;
}

__attribute__((unused)) void free_objects(object_t *obj) {
    switch (obj->type) {
        case OBJ_FUNCTION: {
            object_function_t* function = (object_function_t*)obj;
            free_chunk(&function->chunk);
            FREE(object_function_t, obj);
            break;
        }
        case OBJ_STRING: {
            object_string_t *string = (object_string_t*)obj;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(object_string_t, obj);
            break;
        }
        case OBJ_NATIVE: {
            FREE(object_native_func_t, obj);
            break;
        }
        case OBJ_CLOSURE: {
            object_closure_t* closure = (object_closure_t*)obj;
            FREE_ARRAY(object_upvalue_t*, closure->upvalues, closure->upvalue_count);
            FREE(object_closure_t, obj);
            break;
        }
        case OBJ_UPVALUE: {
            FREE(object_upvalue_t, obj);
            break;
        }
        default: return;
    }
}