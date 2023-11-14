#ifndef __CLOX_VALUE_TABLE_H__
#define __CLOX_VALUE_TABLE_H__

#include "common.h"

#include "utils/table.h"
#include "value/value.h"


bool table_set_value(table_t* table, object_string_t* key, value_t value);
bool table_get_value(table_t* table, object_string_t* key, value_t* value);
bool table_delete_value(table_t* table, object_string_t* key);

void free_table_value(table_t* table);


#endif