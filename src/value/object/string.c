#include <string.h>

#include "common.h"

#include "basic/memory.h"
#include "vm/vm.h"

#include "value/value.h"
#include "value/object.h"
#include "value/object/string.h"

#include "component/valuetable.h"

static uint32_t hash_string(const char* key, int length) {
    uint32_t hash = 216613626u;
    for (int i = 0; i < length; i ++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

static object_string_t* allocate_string(char* chars, int length, uint32_t hash) {
    object_string_t* string = ALLOCATE_OBJECT(object_string_t, OBJ_STRING);
    string->chars = chars;
    string->length = length;
    string->hash = hash;
    table_set_value(&vm.strings, string, NIL_VAL);
    return string;
}

object_string_t* copy_string(const char* chars, int length) {
    uint32_t hash = hash_string(chars, length);

    object_string_t* interned = table_find_string(&vm.strings, chars, length, hash);
    if (interned != NULL) return interned;

    char* heap_chars = ALLOCATE(char, length + 1);
    memcpy(heap_chars, chars, length);
    heap_chars[length] = 0;
    return allocate_string(heap_chars, length, hash);
}

object_string_t* take_string(char* chars, int length) {
    uint32_t hash = hash_string(chars, length);

    object_string_t* interned = table_find_string(&vm.strings, chars, length, hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocate_string(chars, length, hash);
}

object_string_t* concatenate_string(const char* chars, int len, const char* rhs, int r_len) {
    char* nchars = ALLOCATE(char, len + r_len);
    memcpy(nchars, chars, len);
    memcpy(nchars + len, rhs, r_len);
    nchars[len + r_len] = 0;
    return take_string(nchars, len + r_len);
}