#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "constant.h"
#include "error/error.h"

#include "value/value.h"

#include "vm/compiler.h"
#include "vm/scanner.h"
#include "vm/parserules.h"

#include "value/object/string.h"

#ifdef DEBUG_PRINT_CODE
#include "debug/debug.h"
#endif


parser_t parser;
chunk_t* compiling_chunk;


static void error_at(token_t* token, const char* message) {
    if (parser.panic_mode) return;
    parser.panic_mode = true;
    fprintf(stderr, "[line %d] Error", token->line);
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {

    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }
    fprintf(stderr, ": %s\n", message);
    parser.had_error = true;
}

static void error(const char* message) {
    error_at(&parser.previous, message);
}

static void error_at_current(const char* message) {
    error_at(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scan_token();
#ifdef DEBUG_TOKEN_SCANNED
        printf("Current token %d\n", parser.current.type);
#endif
        if (parser.current.type != TOKEN_ERROR) break;

        __CLOX_COMPILER_CURRENT_ERROR(parser.current.start);
    }
}

static void synchronize() {
    parser.panic_mode = false;

    while(parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON) return;
        switch(parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            default:
                ;
        }
        advance();
    }

}

static void consume(tokentype_t type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    __CLOX_COMPILER_CURRENT_ERROR(message);
}

static bool check(tokentype_t type) {
    return parser.current.type == type;
}

static bool match(tokentype_t type) {
    if (!check(type)) return false;
    advance();
    return true;
}

static chunk_t* current_chunk() {
    return compiling_chunk;
}

static void emit_byte(uint8_t byte) {
    write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(uint8_t* bytes, int len) {
    for (int i = 0; i < len; i ++)
        emit_byte(bytes[i]);
}

static void emit_byte_2(uint8_t byte, uint8_t byte2) {
    emit_byte(byte);
    emit_byte(byte2);
}

static void emit_constant(value_t value) {
    int result = write_constant(current_chunk(), value, parser.previous.line);
    if (result) {
        __CLOX_COMPILER_PREVIOUS_ERROR("Too many constants in a chunk. The interpreter can only support at most 65535 constants in a chunk.");
    }
}

static uint16_t make_constant(value_t value) {
    int index = add_constant(current_chunk(), value);
    return index;
}

static void emit_return() {
    emit_byte(OP_RETURN);
}

static void end_compiler() {
    emit_return();
#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error) {
        disassemble_chunk(current_chunk(), "code");
    }
#endif
}

static void parse_precedence(precedence_t precedence) {
    advance();
    parse_fn_t prefix_rule = get_rule(parser.previous.type)->prefix;
    if (prefix_rule == NULL) {
        __CLOX_COMPILER_PREVIOUS_ERROR("Expected expression.");
        return;
    }
    prefix_rule();

    while (precedence <= get_rule(parser.current.type)->precedence) {
        advance();
        parse_fn_t infix_rule = get_rule(parser.previous.type)->infix;
        infix_rule();
    }
}

static void define_variable(uint16_t global) {
    if (global > __OP_CONSTANT_LONG_MAX_INDEX) {
        __CLOX_COMPILER_PREVIOUS_ERROR("Too many constants in a chunk. The interpreter can only support at most 65535 constants in a chunk.");
    } else if (global <= __OP_CONSTANT_MAX_INDEX ) {
        emit_byte_2(OP_DEFINE_GLOBAL, global);
    } else {
        uint8_t hi = (global >> 8) & __UINT8_MASK;
        uint8_t lo = (global     ) & __UINT8_MASK;
        emit_byte(OP_DEFINE_GLOBAL_LONG);
        emit_byte(hi);
        emit_byte(lo);
    }
}

static uint16_t identifier_constant(token_t *name) {
    return make_constant(OBJECT_VAL(copy_string(name->start, name->length)));
}

static uint16_t parse_variable(const char* error_message) {
    consume(TOKEN_IDENTIFIER, error_message);
    return identifier_constant(&parser.previous);
}

static void expression() {
    parse_precedence(PREC_ASSIGNMENT);
}

static void named_variable(token_t name) {
    uint16_t arg = identifier_constant(&name);
    if (arg <= __OP_CONSTANT_MAX_INDEX) {
        uint8_t lo = (arg     ) & __UINT8_MASK;
        emit_byte_2(OP_GET_GLOBAL, lo);
    } else {
        uint8_t hi = (arg >> 8) & __UINT8_MASK;
        uint8_t lo = (arg     ) & __UINT8_MASK;
        emit_byte(OP_GET_GLOBAL_LONG);
        emit_byte(hi);
        emit_byte(lo);
    }
}

/******************** STATEMENTS BEGINS *********************/

static void print_statement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emit_byte(OP_PRINT);
}

static void expression_statement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression");
    emit_byte(OP_POP);
}

/******************** STATEMENTS   ENDS *********************/

static void var_declaration() {
    uint16_t global = parse_variable("Expect variable name.");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emit_byte(OP_NIL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    define_variable(global);
}

static void statement() {
    if (match(TOKEN_PRINT)) {
        print_statement();
    }
    if (match(TOKEN_SEMICOLON)) {}
}

static void declaration() {
    if (match(TOKEN_VAR)) {
        var_declaration();
    } else {
        statement();
    }

    if (parser.panic_mode) synchronize();
}

parse_rule_t* get_rule(tokentype_t tokentype) {
    return &rules[tokentype];
}

void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void number() {
    double value = strtod(parser.previous.start, NULL);
    emit_constant(NUMBER_VAL(value));
}

void unary() {
    tokentype_t operator_type = parser.previous.type;
    expression();

    switch(operator_type) {
        case TOKEN_BANG:
            emit_byte(OP_NOT); break;
        case TOKEN_MINUS:
            emit_byte(OP_NEGATE); break;
        default: return;
    }
}

void binary() {
    tokentype_t operator_type = parser.previous.type;
    parse_rule_t* rule = get_rule(operator_type);
    parse_precedence((precedence_t)(rule->precedence + 1));

    switch(operator_type) {
        case TOKEN_BANG_EQUAL:    emit_byte_2(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emit_byte  (OP_EQUAL); break;
        case TOKEN_GREATER:       emit_byte  (OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emit_byte_2(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emit_byte  (OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emit_byte_2(OP_GREATER, OP_NOT); break;

        case TOKEN_PLUS:          emit_byte  (OP_ADD); break;
        case TOKEN_MINUS:         emit_byte  (OP_SUBSTRACT); break;
        case TOKEN_STAR:          emit_byte  (OP_MULTIPLY); break;
        case TOKEN_SLASH:         emit_byte  (OP_DIVIDE); break;
        default: return;
    }
}

void literal() {
    tokentype_t token_type = parser.previous.type;
    switch(token_type) {
        case TOKEN_NIL:           emit_byte  (OP_NIL); break;
        case TOKEN_TRUE:          emit_byte  (OP_TRUE); break;
        case TOKEN_FALSE:         emit_byte  (OP_FALSE); break;
        default: return;
    }
}

void string(){
    emit_constant(OBJECT_VAL(copy_string(parser.previous.start + 1,
                                parser.previous.length - 2)));
}

void variable() {
    named_variable(parser.previous);
}

bool compile(const char* source, chunk_t* chunk) {
    init_scanner(source);
    compiling_chunk = chunk;

    parser.had_error = false;
    parser.panic_mode = false;

    advance();

    while (!match(TOKEN_EOF)) {
        declaration();
    }

    end_compiler();
    return !parser.had_error;
}