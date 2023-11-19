#ifndef __CLOX_COMPILER_H__
#define __CLOX_COMPILER_H__

#include "vm/vm.h"
#include "vm/scanner.h"

#include "value/object/function.h"

typedef struct struct_parser {
    token_t current;
    token_t previous;
    bool had_error;
    bool panic_mode;
} parser_t;

typedef enum {
    TYPE_FUNCTION,
    TYPE_SCRIPT,
} function_type_t;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_BITWISE,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
} precedence_t;

typedef void (*parse_fn_t)(bool);

typedef struct {
    parse_fn_t prefix;
    parse_fn_t infix;
    precedence_t precedence;
} parse_rule_t;

typedef struct {
    token_t name;
    int depth;
} local_t;

typedef struct {
    uint16_t* offset;
    int count;
    int capacity;
    int start;
} loop_data_t;

typedef struct __compiler {
    struct __compiler* enclosing;

    object_function_t* function;
    function_type_t type;

    local_t      locals[UINT8_COUNT];
    loop_data_t  loops[UINT8_COUNT];
    int local_count;
    int loop_count;
    int scope_depth;
} compiler_t;


object_function_t* compile(const char* source);

#endif