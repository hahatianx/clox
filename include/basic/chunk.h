#ifndef __CLOX_CHUNK_H__
#define __CLOX_CHUNK_H__


#include "common.h"
#include "value/value.h"

typedef enum {
    OP_CONSTANT,          // 2 bytes OP [constant_offset](1 byte )
    OP_CONSTANT_LONG,     // 3 bytes OP [constant_offset](2 bytes)
    OP_NIL,               // 1 byte
    OP_TRUE,
    OP_FALSE,
    OP_NEGATE,            // 1 byte  OP 
    OP_NOT,               // 1 byte   
    OP_ADD,               // 
    OP_SUBSTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_RETURN,            // 1 byte  OP

    OP_DEFINE_GLOBAL,
    OP_DEFINE_GLOBAL_LONG,
    OP_GET_GLOBAL,
    OP_GET_GLOBAL_LONG,
    OP_PRINT,            // internal print, 1 byte
    OP_POP,              // pop the stack
} op_code_t;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    value_array_t constants;
} chunk_t;

void init_chunk      (chunk_t* chunk);
void write_chunk     (chunk_t* chunk, uint8_t byte, int line);
void free_chunk      (chunk_t* chunk);

int add_constant     (chunk_t* chunk, value_t value);
int write_constant   (chunk_t* chunk, value_t value, int line);

int get_line         (chunk_t* chunk, int offset);


#endif