#ifndef __CLOX_VAR_TABLE_H__
#define __CLOX_VAR_TABLE_H__

#include "utils/table.h"

typedef struct {
    bool mutable;
    value_t v;
} var_t;

bool table_set_var   (table_t* table, object_string_t* key, var_t var);
bool table_get_var   (table_t* table, object_string_t* key, var_t* var);
bool table_delete_var(table_t* table, object_string_t* key);

void free_table_var(table_t* table);

#endif