//
// Created by shenshuhan on 1/25/24.
//

#ifndef CLOX_CLASS_H
#define CLOX_CLASS_H

#include "value/value.h"
#include "value/object/string.h"
#include "utils/table.h"

typedef struct clox_klass object_class_t;

struct clox_klass {
    struct clox_object obj;

    object_string_t* name;
};

object_class_t* new_class(object_string_t* name);

#define AS_CLASS(value)    ((object_class_t*)AS_OBJECT(value))

#define IS_CLASS(value)    is_object_type(value, OBJ_CLASS)


typedef struct clox_instance object_instance_t;

struct clox_instance {
    struct clox_object obj;
    object_class_t* klass;

    // This is a value table
    table_t fields;
};

object_instance_t* new_instance(object_class_t* klass);

#define AS_INSTANCE(value) ((object_instance_t*)AS_OBJECT(value))

#define IS_INSTANCE(value) is_object_type(value, OBJ_INSTANCE)


#endif //CLOX_CLASS_H
