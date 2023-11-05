#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "basic/memory.h"
#include "value/object.h"
#include "value/value.h"
#include "utils/table.h"


static table_entry_t* find_entry(table_entry_t* entries, int capacity, object_string_t* key) {
    uint32_t index = key->hash & (capacity - 1);
    table_entry_t* tombstone = NULL;
    for (;;) {
        table_entry_t* entry = &entries[index];
        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                return tombstone != NULL ? tombstone : entry;
            } else {
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            return entry;
        }
        index = (index + 1) & (capacity - 1);
    }
}

static void adjust_capacity(table_t* table, int capacity) {
    table_entry_t* entries = ALLOCATE(table_entry_t, capacity);
    for (int i = 0; i < capacity; i ++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    table->count = 0;
    for (int i = 0; i < table->capacity; i ++) {
        table_entry_t* entry = &table->entries[i];
        if (entry->key == NULL) continue;
        table_entry_t* dest = find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count ++;
    }

    FREE_ARRAY(table_entry_t, table->entries, table->capacity);

    table->entries = entries;
    table->capacity = capacity;
}

void init_table(table_t *table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void free_table(table_t *table) {
    FREE_ARRAY(table_entry_t, table->entries, table->capacity);
    init_table(table);
}

bool table_set(table_t* table, object_string_t* key, value_t value) {
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjust_capacity(table, capacity);
    }

    table_entry_t* entry = find_entry(table->entries, table->capacity, key);
    bool is_new_key = entry->key == NULL;
    if (is_new_key && IS_NIL(entry->value)) table->count ++;

    entry->key = key;
    entry->value = value;
    return is_new_key;
}

void table_add_all(table_t* from, table_t* to) {
    for (int i = 0; i < from->capacity; i ++) {
        table_entry_t* entry = &from->entries[i];
        if (entry->key != NULL) {
            table_set(to, entry->key, entry->value);
        }
    }
}

bool table_get(table_t* table, object_string_t* key, value_t* value) {
    if (table->count == 0) return false;

    table_entry_t* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;

    return true;
}

bool table_delete(table_t* table, object_string_t* key) {
    if (table->count == 0) return false;

    table_entry_t* entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

object_string_t* table_find_string(table_t* table, const char* chars, int length, uint32_t hash) {
    if (table->count == 0) return NULL;

    uint32_t index = hash % table->capacity;
    while (true) {
        table_entry_t* entry = &table->entries[index];
        if (entry->key == NULL) {
            if (IS_NIL(entry->value))
                return NULL;
        } else if (entry->key->length == length && 
                    entry->key->hash == hash &&
                    memcmp(entry->key->chars, chars, length) == 0) {
            return entry->key;
        }
        index = (index + 1) % table->capacity;
    }
}