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

void mark_table_var(table_t* table) {
#ifdef DEBUG_PRINT_TABLE
    printf("number of var table: %d\n", table->count);
    printf("capacity: %d, count: %d\n", table->capacity,  table->count);
#endif
    for (int i = 0; i < table->capacity; i++) {
        table_entry_t *entry = &table->entries[i];
        /*
         *  difference from the book.
         *  In the table here, the value is a pointer instead of an actual value_t
         */
        if (entry->key == NULL || IS_ENTRY_NULL(entry->value))
            continue;
#ifdef DEBUG_PRINT_TABLE
        printf("table number %d, entry %p, key %s, value ", i, entry, ((object_string_t*)entry->key)->chars);
        print_value(((var_t*)entry->value)->v);
        printf("\n");
#endif
        mark_object((object_t*)entry->key);
        mark_value(((var_t*)entry->value)->v);
    }
}