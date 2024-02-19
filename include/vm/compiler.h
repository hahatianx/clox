#ifndef CLOX_COMPILER_H_
#define CLOX_COMPILER_H_

#include "constant.h"

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
    TYPE_METHOD,
    TYPE_INITIALIZER,
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
    bool is_captured;
} local_t;

typedef struct {
    uint16_t* offset;
    int count;
    int capacity;
    int start;
    uint8_t scope_depth;
    bool loop_type;
    /*
     *  true  -> for   loop, scope depth = { [this] for { } }
     *  false -> while loop, scope depth = while { [this] }
     */
} loop_data_t;

typedef struct __compiler {
    struct __compiler* enclosing;

    object_function_t* function;
    function_type_t type;

    upvalue_t    upvalues[UINT8_COUNT];
    local_t      locals[UINT8_COUNT];
    loop_data_t  loops[UINT8_COUNT];
    uint8_t local_count;
    uint8_t loop_count;
    uint8_t scope_depth;
} compiler_t;

typedef struct __class_compiler {
    struct __class_compiler* enclosing;
    bool has_superclass;
} class_compiler_t;


object_function_t* compile(const char* source);
void mark_compiler_roots();

#endif