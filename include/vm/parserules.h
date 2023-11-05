#ifndef __CLOX_PARSE_RULES_H__
#define __CLOX_PARSE_RULES_H__

#include "common.h"

#include "vm/scanner.h"
#include "vm/compiler.h"

void grouping();
void number();
void unary();
void binary();
void literal();
void string();
void variable();

parse_rule_t rules[] = {

    [TOKEN_LEFT_PAREN]     = {grouping, NULL,    PREC_NONE},
    [TOKEN_RIGHT_PAREN]    = {NULL,     NULL,    PREC_NONE},
    [TOKEN_LEFT_BRACE]     = {NULL,     NULL,    PREC_NONE},
    [TOKEN_RIGHT_BRACE]    = {NULL,     NULL,    PREC_NONE},
    [TOKEN_COMMA]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_DOT]            = {NULL,     NULL,    PREC_NONE},
    [TOKEN_MINUS]          = {unary,    binary,  PREC_TERM},
    [TOKEN_PLUS]           = {NULL,     binary,  PREC_TERM},
    [TOKEN_SEMICOLON]      = {NULL,     NULL,    PREC_NONE},
    [TOKEN_SLASH]          = {NULL,     binary,  PREC_FACTOR},
    [TOKEN_STAR]           = {NULL,     binary,  PREC_FACTOR},
    [TOKEN_BANG]           = {unary,    NULL,    PREC_NONE},
    [TOKEN_BANG_EQUAL]     = {NULL,     binary,  PREC_EQUALITY},
    [TOKEN_EQUAL]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_EQUAL_EQUAL]    = {NULL,     binary,  PREC_EQUALITY},
    [TOKEN_GREATER]        = {NULL,     binary,  PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL]  = {NULL,     binary,  PREC_COMPARISON},
    [TOKEN_LESS]           = {NULL,     binary,  PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]     = {NULL,     binary,  PREC_COMPARISON},
    [TOKEN_IDENTIFIER]     = {variable, NULL,    PREC_NONE},
    [TOKEN_STRING]         = {string,   NULL,    PREC_NONE},
    [TOKEN_NUMBER]         = {number,   NULL,    PREC_NONE},
    [TOKEN_AND]            = {NULL,     NULL,    PREC_NONE},
    [TOKEN_CLASS]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_ELSE]           = {NULL,     NULL,    PREC_NONE},
    [TOKEN_FALSE]          = {literal,  NULL,    PREC_NONE},
    [TOKEN_FOR]            = {NULL,     NULL,    PREC_NONE},
    [TOKEN_FUN]            = {NULL,     NULL,    PREC_NONE},
    [TOKEN_IF]             = {NULL,     NULL,    PREC_NONE},
    [TOKEN_NIL]            = {literal,  NULL,    PREC_NONE},
    [TOKEN_OR]             = {NULL,     NULL,    PREC_NONE},
    [TOKEN_PRINT]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_RETURN]         = {NULL,     NULL,    PREC_NONE},
    [TOKEN_SUPER]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_THIS]           = {NULL,     NULL,    PREC_NONE},
    [TOKEN_TRUE]           = {literal,  NULL,    PREC_NONE},
    [TOKEN_VAR]            = {NULL,     NULL,    PREC_NONE},
    [TOKEN_WHILE]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_ERROR]          = {NULL,     NULL,    PREC_NONE},
    [TOKEN_EOF]            = {NULL,     NULL,    PREC_NONE},

};

parse_rule_t* get_rule(tokentype_t tokentype);

#endif