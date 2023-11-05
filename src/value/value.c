#include <stdio.h>


#include "basic/memory.h"

#include "value/value.h"
#include "value/object.h"


void init_value_array(value_array_t* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void write_value_array(value_array_t* array, value_t value) {
    if (array->capacity < array->count + 1) {
        int old_capacity = array->capacity;
        array->capacity = GROW_CAPACITY(old_capacity);
        array->values = GROW_ARRAY(value_t, array->values, old_capacity, array->capacity);
    }
    array->values[array->count ++] = value;
}

void free_value_array(value_array_t* array) {
    FREE_ARRAY(value_t, array->values, array->capacity);
    init_value_array(array);
}

void print_value(value_t value) {
    switch(value.type) {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_NUMBER:
            printf("%g", AS_NUMBER(value));
            break;
        case VAL_OBJ:
            print_object(value);
            break;
    }
}

bool values_equal(value_t a, value_t b) {
    if (a.type != b.type) return false;
    switch(a.type) {
        case VAL_BOOL:    return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:     return true;
        case VAL_NUMBER:  return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ:     return AS_OBJECT(a) == AS_OBJECT(b);
        default:          return false;
    }
}