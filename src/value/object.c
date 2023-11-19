#include <stdio.h>

#include "utils/linklist.h"

#include "basic/memory.h"

#include "value/value.h"
#include "value/object.h"
#include "value/object/string.h"
#include "value/object/function.h"


static int print_function(object_function_t* func) {
    if (!func->name) {
        return printf("<script>");
    }
    return printf("<fn %s>", func->name->chars);
}

#ifdef DEBUG_TRACE_EXECUTION
int print_object(value_t value) {
    switch(OBJ_TYPE(value)) {
        case OBJ_STRING:
            return printf("%s", AS_CSTRING(value));
            break;
        case OBJ_FUNCTION:
            return print_function(AS_FUNCTION(value));
            break;
    }
    return 0;
}
#else
void print_object(value_t value) {
    switch(OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
        case OBJ_FUNCTION:
            print_function(AS_FUNCTION(value));
            break;
    }
}
#endif


void free_objects(object_t *obj) {
    switch (obj->type) {
        case OBJ_FUNCTION: {
            object_function_t* function = (object_function_t*)obj;
            free_chunk(&function->chunk);
            FREE(object_function_t, obj);
        }
        case OBJ_STRING: {
            object_string_t *string = (object_string_t*)obj;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(object_string_t, obj);
            break;
        }
        default: return;
    }
}