#ifndef CLOX_PARSE_RULES_H_
#define CLOX_PARSE_RULES_H_

#include "common.h"

#include "vm/scanner.h"
#include "vm/compiler.h"

void grouping   (bool);
void number     (bool);
void unary      (bool);
void binary     (bool);
void literal    (bool);
void string     (bool);
void variable   (bool);
void _and       (bool);
void _or        (bool);
void call       (bool);
void dot        (bool);
void _index     (bool);
void _this      (bool);
void _super     (bool);
void list       (bool);
void lambda     (bool);

parse_rule_t rules[] = {

    [TOKEN_LEFT_PAREN]     = {grouping, call,    PREC_CALL},
    [TOKEN_RIGHT_PAREN]    = {NULL,     NULL,    PREC_NONE},
    [TOKEN_LEFT_BRACE]     = {NULL,     NULL,    PREC_NONE},
    [TOKEN_RIGHT_BRACE]    = {NULL,     NULL,    PREC_NONE},
    [TOKEN_LEFT_SQUARE]    = {list,     _index,  PREC_CALL},
    [TOKEN_RIGHT_SQUARE]   = {NULL,     NULL,    PREC_NONE},
    [TOKEN_COMMA]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_DOT]            = {NULL,     dot,     PREC_CALL},
    [TOKEN_MINUS]          = {unary,    binary,  PREC_TERM},
    [TOKEN_PLUS]           = {NULL,     binary,  PREC_TERM},
    [TOKEN_SEMICOLON]      = {NULL,     NULL,    PREC_NONE},
    [TOKEN_SLASH]          = {NULL,     binary,  PREC_FACTOR},
    [TOKEN_STAR]           = {NULL,     binary,  PREC_FACTOR},
    [TOKEN_PERCENT]        = {NULL,     binary,  PREC_FACTOR},
    [TOKEN_FLOOR_DIVIDE]   = {NULL,     binary,  PREC_FACTOR},
    [TOKEN_BANG]           = {unary,    NULL,    PREC_NONE},
    [TOKEN_BANG_EQUAL]     = {NULL,     binary,  PREC_EQUALITY},
    [TOKEN_EQUAL]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_EQUAL_EQUAL]    = {NULL,     binary,  PREC_EQUALITY},
    [TOKEN_GREATER]        = {NULL,     binary,  PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL]  = {NULL,     binary,  PREC_COMPARISON},
    [TOKEN_LESS]           = {NULL,     binary,  PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]     = {NULL,     binary,  PREC_COMPARISON},
    [TOKEN_AMPERSAND]      = {NULL,     binary,  PREC_BITWISE},
    [TOKEN_CAP]            = {NULL,     binary,  PREC_BITWISE},
    [TOKEN_PIPE]           = {NULL,     binary,  PREC_BITWISE},
    [TOKEN_LEFT_SHIFT]     = {NULL,     binary,  PREC_BITWISE},
    [TOKEN_RIGHT_SHIFT]    = {NULL,     binary,  PREC_BITWISE},  
    [TOKEN_IDENTIFIER]     = {variable, NULL,    PREC_NONE},
    [TOKEN_STRING]         = {string,   NULL,    PREC_NONE},
    [TOKEN_INTEGER]        = {number,   NULL,    PREC_NONE},
    [TOKEN_NUMBER]         = {number,   NULL,    PREC_NONE},
    [TOKEN_LAMBDA]         = {lambda,   NULL,    PREC_NONE},
    [TOKEN_AND]            = {NULL,     _and,    PREC_AND},
    [TOKEN_CLASS]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_ELSE]           = {NULL,     NULL,    PREC_NONE},
    [TOKEN_FALSE]          = {literal,  NULL,    PREC_NONE},
    [TOKEN_FOR]            = {NULL,     NULL,    PREC_NONE},
    [TOKEN_FUN]            = {NULL,     NULL,    PREC_NONE},
    [TOKEN_IF]             = {NULL,     NULL,    PREC_NONE},
    [TOKEN_NIL]            = {literal,  NULL,    PREC_NONE},
    [TOKEN_OR]             = {NULL,     _or,     PREC_OR},
    [TOKEN_PRINT]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_RETURN]         = {NULL,     NULL,    PREC_NONE},
    [TOKEN_SUPER]          = {_super,   NULL,    PREC_NONE},
    [TOKEN_THIS]           = {_this,    NULL,    PREC_NONE},
    [TOKEN_TRUE]           = {literal,  NULL,    PREC_NONE},
    [TOKEN_VAR]            = {NULL,     NULL,    PREC_NONE},
    [TOKEN_WHILE]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_ERROR]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_EOF]            = {NULL,     NULL,    PREC_NONE},

};

parse_rule_t* get_rule(tokentype_t tokentype);

#endif