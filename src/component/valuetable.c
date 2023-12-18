#include <string.h>

#include "common.h"

#include "component/valuetable.h"


bool table_set_value(table_t* table, object_string_t* key, value_t value) {
    value_t* v = ALLOCATE(value_t, 1);
    memcpy(v, &value, sizeof(value_t));
    return table_set(table, key, (void*) v);
}

__attribute__((unused)) bool table_get_value(table_t* table, object_string_t* key, value_t* value) {
    void* v = NULL;
    bool result = table_get(table, key, &v);
    if (result)
        memcpy(value, (value_t*) v, sizeof(value_t));
    return result;
}

__attribute__((unused)) bool table_delete_value(table_t* table, object_string_t* key) {
    void* return_ptr = NULL;
    bool result = table_delete(table, key, &return_ptr);
    if (result) {
        FREE(value_t, (value_t*) return_ptr);
    }
    return result;
}

void free_table_value(table_t* table) {
    for (int i = 0; i < table->capacity; i ++) {
        table_entry_t* entry = &table->entries[i];
        if (entry->key != NULL) {
            FREE(value_t, (value_t*) entry->value);
            entry->value = NULL;
        }
    }
    free_table(table);
}

// TODO: think about a way to mark all tables
void mark_table_value(table_t* table) {
    for (int i = 0; i < table->capacity; i++) {
        table_entry_t* entry = &table->entries[i];
        if (entry->key == NULL || IS_ENTRY_NULL(entry->value))
            continue;
        mark_object((object_t*)entry->key);
        mark_value(*((value_t*)entry->value));
    }
}