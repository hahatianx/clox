//
// Created by shenshuhan on 1/28/24.
//
#include <string.h>

#include "constant.h"

#include "value/object/list.h"
#include "value/object.h"

object_list_t* new_list(uint32_t capacity, value_t value) {
    if (capacity > LIST_CAPACITY_MAX)
        return NULL;

    object_list_t *list = ALLOCATE_OBJECT(object_list_t, OBJ_LIST);
    list->capacity = capacity;
    list->initial = value;
    value_t* value_list = ALLOCATE(value_t, list->capacity);
    memset(value_list, 0, list->capacity * sizeof(value_t));
    list->list = value_list;
    return list;
}

int get_list_value(object_list_t* list, uint32_t index, value_t* value) {
    if (index >= list->capacity) {
        /*
         *  out-of-bound
         */
        return -1;
    }
    int addr = *((int*)&(list->list[index]));
    *value = addr ? list->list[index] : list->initial;
    return 0;
}

int set_list_value(object_list_t* list, uint32_t index, value_t value) {
    if (index >= list->capacity) {
        return -1;
    }
    list->list[index] = value;
    return 0;
}