//
// Created by shenshuhan on 1/28/24.
//

#ifndef CLOX_LIST_H
#define CLOX_LIST_H

#include "value/value.h"

typedef struct clox_list object_list_t;

struct clox_list {
    struct clox_object obj;

    /*
     * If unsigned, return this value when fetched
     */
    value_t initial;
    int capacity;
    value_t* list;
};

#define IS_LIST(value) is_object_type(value, OBJ_LIST)

#define AS_LIST(value) ((object_list_t*)AS_OBJECT(value))

object_list_t* new_list(uint32_t capacity, value_t value);

int  get_list_value(object_list_t* list, uint32_t index, value_t *value);
int  set_list_value(object_list_t* list, uint32_t index, value_t value);

#endif //CLOX_LIST_H
