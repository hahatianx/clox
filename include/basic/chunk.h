#ifndef CLOX_CHUNK_H_
#define CLOX_CHUNK_H_


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
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MOD,
    OP_FLOOR_DIVIDE,
    OP_LEFT_SHIFT,
    OP_RIGHT_SHIFT,
    OP_BIT_AND,
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,

    OP_JUMP_IF_FALSE,     // conditional jump forward
    OP_JUMP,              // jump forward
    OP_LOOP,              // jump back
    OP_CALL,
    OP_CLOSURE,
    OP_CLOSURE_UPVALUE,
    OP_RETURN,            // 1 byte  OP

    OP_DEFINE_GLOBAL,
    OP_DEFINE_GLOBAL_LONG,
    OP_DEFINE_MUT_GLOBAL,
    OP_DEFINE_MUT_GLOBAL_LONG,
    OP_GET_GLOBAL,
    OP_GET_GLOBAL_LONG,
    OP_SET_GLOBAL,
    OP_SET_GLOBAL_LONG,
    OP_DEFINE_LOCAL,
    OP_DEFINE_MUT_LOCAL,
    OP_SET_LOCAL,
    OP_GET_LOCAL,
    OP_GET_SUPER,
    OP_GET_SUPER_LONG,
    OP_SET_UPVALUE,
    OP_GET_UPVALUE,
    OP_GET_PROPERTY,
    OP_GET_PROPERTY_LONG,
    OP_SET_PROPERTY,
    OP_SET_PROPERTY_LONG,
    OP_GET_ARRAY_INDEX,
    OP_SET_ARRAY_INDEX,

    OP_PRINT,            // internal print, 1 byte
    OP_PRINTLN,
    OP_POP,              // pop the stack
    OP_POPN,

    OP_ARRAY,

    OP_INHERIT,
    OP_INVOKE,
    OP_INVOKE_LONG,
    OP_CLASS,
    OP_CLASS_LONG,
    OP_METHOD,
    OP_METHOD_LONG,
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