#ifndef CLOX_VM_H_
#define CLOX_VM_H_

#include "utils/linklist.h"
#include "utils/table.h"
#include "utils/stack.h"

#include "value/value.h"
#include "value/object/function.h"

#include "basic/chunk.h"

#define FRAMES_MAX 128
#define STACK_MAX (FRAMES_MAX * UINT8_MAX)

typedef struct {
    bool mutable;
} var_metadata_t;

typedef struct {
    object_closure_t *closure;
    uint8_t* ip;
    value_t* slots;
    var_metadata_t* local_meta;
} callframe_t;

typedef struct {
    callframe_t    frames [FRAMES_MAX];
    int frame_count;

    value_t        stack  [STACK_MAX];
    var_metadata_t local  [STACK_MAX];
    value_t* stack_top;

    // the string table is a value table, but without values
    table_t strings;
    // the globals table is a var table
    table_t globals;
    list_t obj;
    list_t open_upvalues;

    // gray stack
    clox_stack_t gray_stack;
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

// housekeeping
void remove_unused_strings();


#endif