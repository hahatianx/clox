#ifndef __CLOX_COMPILER_H__
#define __CLOX_COMPILER_H__

#include "vm/vm.h"

#include "vm/scanner.h"


typedef struct struct_parser {
    token_t current;
    token_t previous;
    bool had_error;
    bool panic_mode;
} parser_t;

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


typedef void (*parse_fn_t)();

typedef struct {
    parse_fn_t prefix;
    parse_fn_t infix;
    precedence_t precedence;
} parse_rule_t;


bool compile(const char* source, chunk_t* chunk);

#endif