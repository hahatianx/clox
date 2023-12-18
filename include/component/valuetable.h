#ifndef CLOX_VALUE_TABLE_H_
#define CLOX_VALUE_TABLE_H_

#include "common.h"

#include "utils/table.h"
#include "value/value.h"


bool table_set_value(table_t* table, object_string_t* key, value_t value);

__attribute__((unused)) bool table_get_value(table_t* table, object_string_t* key, value_t* value);

__attribute__((unused)) bool table_delete_value(table_t* table, object_string_t* key);

void free_table_value(table_t* table);
void mark_table_value(table_t* table);


#endif