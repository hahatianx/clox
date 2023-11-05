#ifndef __CLOX_OBJECT_STRING_H__
#define __CLOX_OBJECT_STRING_H__

#include "value/object.h"

typedef struct clox_string object_string_t;

struct clox_string {
    struct clox_object obj;
    int length;
    uint32_t hash;
    char* chars;
};

#define IS_STRING(value)   is_object_type(value, OBJ_STRING)

#define AS_STRING(value)   ((object_string_t*)AS_OBJECT(value))
#define AS_CSTRING(value)  (((object_string_t*)AS_OBJECT(value))->chars)

object_string_t* copy_string(const char* chars, int length);
object_string_t* take_string(char* chars, int length);
object_string_t* concatenate_string(const char* chars, int len, const char* rhs, int r_len);

#endif