#ifndef __CLOX_VM_H__
#define __CLOX_VM_H__

#include "utils/linklist.h"
#include "utils/table.h"

#include "value/value.h"
#include "basic/chunk.h"

#define STACK_MAX 256

typedef struct {
    chunk_t* chunk;
    uint8_t* ip;
    value_t stack[STACK_MAX];
    value_t* stack_top;
    table_t strings;
    table_t globals;
    list_t obj;
} vm_t;

extern vm_t vm;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} interpret_result_t;

void init_vm();
void free_vm();
interpret_result_t interpret(const char* source);

void    push(value_t value);
value_t pop();


#endif