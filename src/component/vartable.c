#include <string.h>

#include "component/vartable.h"

bool table_set_var(table_t* table, object_string_t* key, var_t var) {
    var_t* v = ALLOCATE(var_t, 1);
    memcpy(v, &var, sizeof(var_t));
    return table_set(table, key, (void*) v);
}

bool table_get_var(table_t* table, object_string_t* key, var_t* var) {
    void* v = NULL;
    bool result = table_get(table, key, &v);
    if (result)
        memcpy(var, (var_t*) v, sizeof(var_t));
    return result;
}

__attribute__((unused)) bool table_delete_var(table_t* table, object_string_t* key) {
    void* v = NULL;
    bool result = table_delete(table, key, &v);
    if (result) {
        FREE(var_t, v);
    }
    return result;
}

void free_table_var(table_t* table) {
    for (int i = 0; i < table->count; ++i) {
        table_entry_t* entry = &table->entries[i];
        if (entry->key != NULL) {
            FREE(var_t, (var_t*) entry->value);
            entry->value = NULL;
        }
    }
    free_table(table);
}