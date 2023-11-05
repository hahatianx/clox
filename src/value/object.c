#include <stdio.h>

#include "utils/linklist.h"

#include "basic/memory.h"

#include "value/value.h"
#include "value/object.h"
#include "value/object/string.h"

void print_object(value_t value) {
    switch(OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;

    }
}

void free_objects(object_t *obj) {
    switch (obj->type) {
        case OBJ_STRING: {
            object_string_t *string = (object_string_t*)obj;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(object_string_t, obj);
            break;
        }
        default: return;
    }
}