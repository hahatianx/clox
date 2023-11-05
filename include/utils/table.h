#ifndef __CLOX_TABLE_H__
#define __CLOX_TABLE_H__

#include "common.h"
#include "value/value.h"
#include "value/object/string.h"

#define TABLE_MAX_LOAD 0.75

typedef struct {
    object_string_t* key;
    value_t value;
} table_entry_t;

typedef struct {
    int count;
    int capacity;
    table_entry_t* entries;
} table_t;

void init_table   (table_t* table);
void free_table   (table_t* table);

bool table_set    (table_t* table, object_string_t* key, value_t value);
bool table_get    (table_t* table, object_string_t* key, value_t* value);
bool table_delete (table_t* table, object_string_t* key);
void table_add_all(table_t* from, table_t* to);

object_string_t* table_find_string(table_t* table, const char* chars, int length, uint32_t hash);

#endif