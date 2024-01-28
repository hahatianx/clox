//
// Created by shenshuhan on 1/25/24.
//

#include "value/object/class.h"

object_class_t* new_class(object_string_t* name) {
    object_class_t* klass = ALLOCATE_OBJECT(object_class_t, OBJ_CLASS);
    klass->name = name;
    return klass;
}

object_instance_t* new_instance(object_class_t* klass) {
    object_instance_t* instance = ALLOCATE_OBJECT(object_instance_t, OBJ_INSTANCE);
    instance->klass = klass;
    init_table(&instance->fields);
    return instance;
}