#ifndef __CLOX_VALUE_H__
#define __CLOX_VALUE_H__

#include "common.h"

#include "utils/linklist.h"

typedef enum {
    OBJ_STRING,
} object_type_t;

typedef struct clox_object {
    object_type_t type;
    list_link_t link;
} object_t;

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_INT,
    VAL_FLOAT,
    VAL_OBJ,
} value_type_t;

typedef struct {
    value_type_t type;
    union {
        bool boolean;
        double number;
        int64_t integer;
        object_t* obj;
    } as;
} value_t;

#define IS_INT(value)      ((value).type == VAL_INT)
#define IS_BOOL(value)     ((value).type == VAL_BOOL)
#define IS_NIL(value)      ((value).type == VAL_NIL)
#define IS_FLOAT(value)    ((value).type == VAL_FLOAT)
#define IS_OBJECT(value)   ((value).type == VAL_OBJ)
#define IS_NUMBER(value)   ((value).type == VAL_FLOAT || (value).type == VAL_INT)

#define AS_INT(value)      ((value).as.integer)
#define AS_BOOL(value)     ((value).as.boolean)
#define AS_FLOAT(value)    ((value).as.number)
#define AS_OBJECT(value)   ((value).as.obj)
/*
    AS_NUMBER(value) must not be a macro !!
    * value is called multiple times within the block *
*/
double AS_NUMBER(value_t value);

#define BOOL_VAL(value)    ((value_t) {VAL_BOOL, {.boolean = value}})
#define NIL_VAL            ((value_t) {VAL_NIL,  {.number = 0}})
#define INT_VAL(value)     ((value_t) {VAL_INT,  {.integer = value}})
#define FLOAT_VAL(value)   ((value_t) {VAL_FLOAT, {.number = value}})
#define OBJECT_VAL(object) ((value_t) {VAL_OBJ, {.obj = (object_t*)object}})


typedef struct {
    int capacity;
    int count;
    value_t* values;
} value_array_t;



bool values_equal(value_t a, value_t b);

void init_value_array (value_array_t* array);
void write_value_array(value_array_t* array, value_t value);
void free_value_array (value_array_t* array);

#ifdef DEBUG_PRINT_CODE
int print_value      (value_t value);
#else
void print_value     (value_t value);
#endif


#endif