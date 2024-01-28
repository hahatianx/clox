#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "constant.h"
#include "switch.h"

#include "error/error.h"

#include "vm/compiler.h"
#include "vm/scanner.h"
#include "vm/parserules.h"

#include "value/value.h"
#include "value/object/string.h"
#include "value/object/function.h"

#ifdef DEBUG_PRINT_CODE
#include "debug/debug.h"
#endif


parser_t parser;
compiler_t* current = NULL;
chunk_t* compiling_chunk;

static void block();
static void statement();
static void var_declaration();
static void begin_scope();
static void end_scope();

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
    return &current->function->chunk;
}

static void emit_byte(uint8_t byte) {
    write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_byte_2(uint8_t byte, uint8_t byte2) {
    emit_byte(byte);
    emit_byte(byte2);
}

static void emit_constant(value_t value) {
    int result = write_constant(current_chunk(), value, parser.previous.line);
    if (result) {
        __CLOX_COMPILER_PREVIOUS_ERROR("too many constants in a chunk. The interpreter can only support at most 65535 constants in a chunk.");
    }
}

static int emit_jump(uint8_t op) {
    emit_byte(op);
    emit_byte(__UINT8_MASK);
    emit_byte(__UINT8_MASK);
    return current_chunk()->count - 2;
}

static void patch_jump(int offset) {
    int jump_step = current_chunk()->count - offset - 2;
    if (jump_step > UINT16_MAX) {
        __CLOX_ERROR("This is a clox limitation. Cannot jump over too much.");
    }
    current_chunk()->code[offset    ] = (jump_step >> 8) & __UINT8_MASK;
    current_chunk()->code[offset + 1] = (jump_step     ) & __UINT8_MASK;
}

static void emit_loop(int offset) {
    int jump_step = current_chunk()->count - offset + 3;
    if (jump_step > UINT16_MAX) {
        __CLOX_ERROR("This is a clox limitation. Cannot loop over too much.");
    }
    emit_byte(OP_LOOP);
    emit_byte_2(
        (jump_step >> 8) & __UINT8_MASK,
        (jump_step     ) & __UINT8_MASK);
}

static void init_compiler(compiler_t* compiler, function_type_t type) {
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;

    compiler->local_count = 0;
    compiler->loop_count  = 0;
    compiler->scope_depth = 0;
    compiler->function = new_function();
    current = compiler;

    if (type != TYPE_SCRIPT)
        current->function->name = copy_string(parser.previous.start, parser.previous.length);

    local_t* local = &current->locals[current->local_count++];
    local->depth = 0;
    local->is_captured = false;
    local->name.start = "";
    local->name.length = 0;
}

static uint16_t make_constant(value_t value) {
    int index = add_constant(current_chunk(), value);
    return index;
}

static void emit_return() {
    emit_byte(OP_NIL);
    emit_byte(OP_RETURN);
}

static object_function_t* end_compiler() {
    emit_return();
    object_function_t* func = current->function;

#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error) {
        disassemble_chunk(current_chunk(), func->name ? func->name->chars : "<script>");
    }
#endif
    current = current->enclosing;
    return func;
}

static void parse_precedence(precedence_t precedence) {
    advance();
    bool can_assign = precedence <= PREC_ASSIGNMENT;
    parse_fn_t prefix_rule = get_rule(parser.previous.type)->prefix;
    if (prefix_rule == NULL) {
        __CLOX_COMPILER_PREVIOUS_ERROR("Expected expression.");
        return;
    }
    prefix_rule(can_assign);

    while (precedence <= get_rule(parser.current.type)->precedence) {
        advance();
        parse_fn_t infix_rule = get_rule(parser.previous.type)->infix;
        infix_rule(can_assign);
    }

    if (can_assign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

static void mark_variable_inited() {
    if (!current->scope_depth) return;
    current->locals[current->local_count - 1].depth =
        current->scope_depth;
#ifdef DEBUG_PRINT_CODE 
    if (!parser.had_error) {
        disassemble_locals(current, "local vars");
    }
#endif
}

static void define_variable(uint16_t global, bool mutable) {
    if (current->scope_depth) {
        mark_variable_inited();
        emit_byte(mutable ? OP_DEFINE_MUT_LOCAL : OP_DEFINE_LOCAL);
        return;
    }
    if (global > __OP_CONSTANT_LONG_MAX_INDEX) {
        __CLOX_COMPILER_PREVIOUS_ERROR("too many constants in a chunk. The interpreter can only support at most 65535 constants in a chunk.");
    } else if (global <= __OP_CONSTANT_MAX_INDEX ) {
        emit_byte_2(mutable ?  OP_DEFINE_MUT_GLOBAL : OP_DEFINE_GLOBAL, global);
    } else {
        uint8_t hi = (global >> 8) & __UINT8_MASK;
        uint8_t lo = (global     ) & __UINT8_MASK;
        emit_byte(mutable ? OP_DEFINE_MUT_GLOBAL_LONG : OP_DEFINE_GLOBAL_LONG);
        emit_byte(hi);
        emit_byte(lo);
    }
}

static uint16_t identifier_constant(token_t *name) {
    return make_constant(OBJECT_VAL(copy_string(name->start, name->length)));
}

static bool identifier_equal(token_t* a, token_t* b) {
    if (a->length != b->length) return false;
    return !memcmp(a->start, b->start, a->length);
}

static int resolve_local(compiler_t* compiler, token_t* name) {
    for (int i = compiler->local_count - 1; i >= 0; --i) {
        local_t* local = &compiler->locals[i];
        if (identifier_equal(name, &local->name)) {
            if (local->depth == -1) {
                __CLOX_COMPILER_PREVIOUS_ERROR("can't read local variable in its own initializer.");
            }
            return i;
        }
    }
    return -1;
}

static int add_upvalue(compiler_t* compiler, uint8_t index, bool is_local) {
    int upvalue_count = compiler->function->upvalue_count;

    for (int i = 0; i < upvalue_count; i++) {
        upvalue_t* upvalue = &compiler->function->upvalues[i];
        if (upvalue->index == index && upvalue->is_local == is_local) {
            return i;
        }
    }

    if (upvalue_count == UINT8_COUNT) {
        __CLOX_ERROR("Too many closure variables in function.");
    }

    compiler->upvalues[upvalue_count].is_local = is_local;
    compiler->upvalues[upvalue_count].index = index;
    return compiler->function->upvalue_count++;
}

static int resolve_upvalue(compiler_t* compiler, token_t* name) {
    if (compiler->enclosing == NULL) return -1;

    int local = resolve_local(compiler->enclosing, name);
    if (~local) {
        compiler->enclosing->locals[local].is_captured = true;
        return add_upvalue(compiler, (uint8_t)local, true);
    }

    int upvalue = resolve_upvalue(compiler->enclosing, name);
    if (~upvalue) {
        return add_upvalue(compiler, (uint8_t)upvalue, false);
    }

    return -1;
}

static void add_local(token_t name) {
    if(current->local_count == UINT8_COUNT) {
        __CLOX_COMPILER_PREVIOUS_ERROR("too many local variables in function.");
    }
    local_t* local = &current->locals[current->local_count++];
    local->name = name;
    local->depth = -1;
    local->is_captured = false;
}

static void declare_variable() {
    if (current->scope_depth == 0) return;
    token_t* name = &parser.previous;
    for (int i = current->local_count - 1; i >= 0; --i) {
        local_t* local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scope_depth) {
            break;
        }

        if (identifier_equal(name, &local->name)) {
            __CLOX_COMPILER_PREVIOUS_ERROR("already a variable with this name in this scope.");
        }
    } 
    add_local(*name);
}

static uint16_t parse_variable(const char* error_message) {
    consume(TOKEN_IDENTIFIER, error_message);

    declare_variable();
    if (current->scope_depth) return 0;
    return identifier_constant(&parser.previous);
}

static void expression() {
    parse_precedence(PREC_ASSIGNMENT);
}

static void named_variable(token_t name, bool can_assign) {
    int arg = resolve_local(current, &name);
    if (arg != -1) {
        if (can_assign && match(TOKEN_EQUAL)) {
            expression();
            emit_byte_2(OP_SET_LOCAL, (uint8_t) arg);
        } else {
            emit_byte_2(OP_GET_LOCAL, (uint8_t) arg);
        }
    } else if (~(arg = resolve_upvalue(current, &name))) {
        if (can_assign && match(TOKEN_EQUAL)) {
            expression();
            emit_byte_2(OP_SET_UPVALUE, (uint8_t) arg);
        } else {
            emit_byte_2(OP_GET_UPVALUE, (uint8_t) arg);
        }
    } else {
        arg = identifier_constant(&name);
        if (can_assign && match(TOKEN_EQUAL)) {
            expression();
            if (arg <= __OP_CONSTANT_MAX_INDEX) {
                uint8_t lo = (arg     ) & __UINT8_MASK;
                emit_byte_2(OP_SET_GLOBAL, lo);
            } else {
                uint8_t hi = (arg >> 8) & __UINT8_MASK;
                uint8_t lo = (arg     ) & __UINT8_MASK;
                emit_byte(OP_SET_GLOBAL_LONG);
                emit_byte(hi);
                emit_byte(lo);
            }
        } else {
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
    }

}

/********************                  **********************/

static void begin_loop_scope(int start) {
    int now = current->loop_count;
    current->loops[now].count = 0;
    current->loops[now].capacity = 2;
    current->loops[now].offset = ALLOCATE(uint16_t, 2);

    current->loops[now].start = start;

    current->loop_count++;
}

static void end_loop_scope() {
    current->loop_count--;
    int now = current->loop_count;
    FREE_ARRAY(uint16_t, current->loops[now].offset, current->loops[now].capacity);
}

static loop_data_t* current_loop_context() {
    if (!current->loop_count) {
        __CLOX_COMPILER_PREVIOUS_ERROR("it's not in any loop structure.");
    }
    return &current->loops[current->loop_count - 1];
}

static void insert_offset(uint16_t line) {
    loop_data_t* cur = current_loop_context();
    if (cur->capacity < cur->count + 1) {
        int old_capacity = cur->capacity;
        cur->capacity = GROW_CAPACITY(old_capacity);
        cur->offset = GROW_ARRAY(uint16_t, cur->offset, old_capacity, cur->capacity);
    }
    cur->offset[cur->count] = line;
    cur->count++;
}

static void patch_loop_jumps() {
    uint16_t* offset = current_loop_context()->offset;
    for (int i = 0; i < current_loop_context()->count; offset++, i++) {
        patch_jump(*offset);
    }
}

/******************** STATEMENTS BEGINS *********************/

static void print_statement(token_t token) {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emit_byte(token.type == TOKEN_PRINT ? OP_PRINT : OP_PRINTLN);
}

static void expression_statement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression");
    emit_byte(OP_POP);
}

static void if_statement() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int then_jump = emit_jump(OP_JUMP_IF_FALSE);

    emit_byte(OP_POP);
    statement();

    int else_jump = emit_jump(OP_JUMP);
    patch_jump(then_jump);
    emit_byte(OP_POP);
    if (match(TOKEN_ELSE)) statement();
    patch_jump(else_jump);
}

static void break_statement() {
    current_loop_context();
    if (parser.had_error) return;
    consume(TOKEN_SEMICOLON, "Expect ';' after 'break'.");
    int offset = emit_jump(OP_JUMP);
    insert_offset(offset);
}

static void continue_statement() {
    current_loop_context();
    if (parser.had_error) return;
    consume(TOKEN_SEMICOLON, "Expect ';' after 'continue'.");
    loop_data_t* cur = current_loop_context();
    emit_loop(cur->start);
}

static void while_statement() {
    int loop_start = current_chunk()->count;
    begin_loop_scope(loop_start);

    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int end_loop = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);

    /*
     *      WARN: the behavior of variable scope is different
     *   {  statements; } has a separate variable scope for each loop.
     *      statement     itself shares the scope with the outer function scope
     *    the closure behaves differently
     *    please refer to p494 DESIGN NOTE: CLOSING OVER THE LOOP VARIABLE
     */
    statement();

    emit_loop(loop_start);
    patch_jump(end_loop);
    emit_byte(OP_POP);
    patch_loop_jumps();

    end_loop_scope();
}

static void for_statement() {
    begin_scope();
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    if (match(TOKEN_SEMICOLON)) {
        ;
    } else if (match(TOKEN_VAR)) {
        var_declaration();
    } else {
        expression_statement();
    }

    int loop_start = current_chunk()->count;

    int exit_loop = -1;
    if (!match(TOKEN_SEMICOLON)) {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        exit_loop = emit_jump(OP_JUMP_IF_FALSE);
        emit_byte(OP_POP);
    }

    int inc_start = -1;
    if (!match(TOKEN_RIGHT_PAREN)) {
        int stmt_start = emit_jump(OP_JUMP);
        inc_start = current_chunk()->count;
        expression();

        emit_byte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emit_loop(loop_start);
        patch_jump(stmt_start);
    }

    begin_loop_scope(~inc_start ? inc_start : loop_start);

    /*
     *      WARN: the behavior of variable scope is different
     *   {  statements; } has a separate variable scope for each loop.
     *      statement     itself shares the scope with the outer function scope
     *    the closure behaves differently
     *    please refer to p494 DESIGN NOTE: CLOSING OVER THE LOOP VARIABLE
     */
    statement();
    emit_loop(~inc_start ? inc_start : loop_start);
    if (~exit_loop) {
        patch_jump(exit_loop);
        emit_byte(OP_POP);
    }
    patch_loop_jumps();

    end_loop_scope();
    end_scope();
}

static void return_statement() {
    if (current->type == TYPE_SCRIPT) {
        __CLOX_COMPILER_PREVIOUS_ERROR("Can't return from top-level code.");
    }
    if (match(TOKEN_SEMICOLON)) {
        emit_return();
    } else {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
        emit_byte(OP_RETURN);
    }
}

/******************** STATEMENTS   ENDS *********************/
/******************** DECLARATIONS STARTS *******************/

static void function(function_type_t type) {
    compiler_t compiler;
    init_compiler(&compiler, type);
    begin_scope();

    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");

    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            current->function->arity++;
            if (current->function->arity > 255) {
                __CLOX_COMPILER_CURRENT_ERROR("Can't have more than 255 parameters.");
            }
            bool mutable = false;
            if (match(TOKEN_MUT)) mutable = true;
            uint16_t constant = parse_variable("Expect parameter name.");
            define_variable(constant, mutable);
        } while(match(TOKEN_COMMA));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    block();

    object_function_t* func = end_compiler();
    emit_byte_2(OP_CLOSURE, make_constant(OBJECT_VAL(func)));
    for (int i = 0; i < func->upvalue_count; ++i) {
        emit_byte_2(compiler.upvalues[i].is_local ? 1 : 0, compiler.upvalues[i].index);
    }
}

static void compile_lambda(function_type_t type) {
    compiler_t compiler;
    init_compiler(&compiler, type);
    begin_scope();
    consume(TOKEN_LEFT_PAREN, "Expect '(' after keyword 'lambda'.");

    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            current->function->arity++;
            if (current->function->arity > 255) {
                __CLOX_COMPILER_CURRENT_ERROR("Can't have more than 255 parameters.");
            }
            bool mutable = false;
            if (match(TOKEN_MUT)) mutable = true;
            uint16_t constant = parse_variable("Expect parameter name.");
            define_variable(constant, mutable);
        } while(match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    consume(TOKEN_ARROW, "Expect '=>' between lambda parameters and lambda body.");

    if (match(TOKEN_LEFT_BRACE))
        block();
    else {
        expression();
        emit_byte(OP_RETURN);
    }

    object_function_t* func = end_compiler();
    emit_byte_2(OP_CLOSURE, make_constant(OBJECT_VAL(func)));
    for (int i = 0; i < func->upvalue_count; ++i) {
        emit_byte_2(compiler.upvalues[i].is_local ? 1 : 0, compiler.upvalues[i].index);
    }
}

static void var_declaration() {
    bool mutable = false;
    if (match(TOKEN_MUT)) {
        mutable = true;
    }

    uint16_t global = parse_variable("Expect variable name.");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else if (!mutable) {
        __CLOX_COMPILER_PREVIOUS_ERROR("immutable variables must be initialized.");
    } else {
        emit_byte(OP_NIL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    define_variable(global, mutable);
}

static void fun_declaration() {
    uint16_t global = parse_variable("Expect function name.");
    mark_variable_inited();
    function(TYPE_FUNCTION);
    define_variable(global, false);
}

static void class_declaration() {
    consume(TOKEN_IDENTIFIER, "Expect class name.");
    uint16_t name_constant = identifier_constant(&parser.previous);

    declare_variable();

    if (name_constant <= __OP_CONSTANT_MAX_INDEX) {
        emit_byte_2(OP_CLASS, name_constant & __UINT8_MASK);
    } else {
        emit_byte(OP_CLASS_LONG);
        uint8_t hi = (name_constant >> 8) & __UINT8_MASK;
        uint8_t lo = (name_constant     ) & __UINT8_MASK;
        emit_byte_2(hi, lo);
    }

    define_variable(name_constant, false);

    consume(TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after class body.");

}

/******************** DECLARATIONS ENDS *********************/

static void begin_scope() {
    current->scope_depth++;
}

static void pop_stack_values(uint8_t *count) {
    if (*count == 1) emit_byte(OP_POP);
    else if (*count > 1) emit_byte_2(OP_POPN, *count);
    *count = 0;
}

static void end_scope() {
    current->scope_depth--;
    uint8_t pop_count = 0;
    while (current->local_count > 0 &&
        current->locals[current->local_count - 1].depth > current->scope_depth) {
        if (current->locals[current->local_count - 1].is_captured) {
            /*  clean up pop_count first */
            pop_stack_values(&pop_count);
            emit_byte(OP_CLOSURE_UPVALUE);
        } else {
            pop_count++;
        }
        current->local_count--;
    }
    pop_stack_values(&pop_count);
}

static void statement() {
    if (match(TOKEN_PRINT) || match(TOKEN_PRINTLN)) {
        print_statement(parser.previous);
    } else if (match(TOKEN_SEMICOLON)) {
        ;
    } else if (match(TOKEN_LEFT_BRACE)) {
        begin_scope();
        block();
        end_scope();
    } else if (match(TOKEN_IF)) {
        if_statement();
    } else if (match(TOKEN_WHILE)) {
        while_statement();
    } else if (match(TOKEN_FOR)) {
        for_statement();
    } else if (match(TOKEN_BREAK)) {
        break_statement();
    } else if (match(TOKEN_CONTINUE)) {
        continue_statement();
    } else if (match(TOKEN_RETURN)) {
        return_statement();
    } else {
        expression_statement();
    }
}

static void declaration() {
    if (match(TOKEN_FUN)) {
        fun_declaration();
    } else if (match(TOKEN_VAR)) {
        var_declaration();
    } else if (match(TOKEN_CLASS)) {
        class_declaration();
    } else {
        statement();
    }

    if (parser.panic_mode) synchronize();
}


static void block() {
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        declaration();
    }
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

parse_rule_t* get_rule(tokentype_t tokentype) {
    return &rules[tokentype];
}

void grouping(bool can_assign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void number(bool can_assign) {
    value_t value;
    if (parser.previous.type == TOKEN_INTEGER) {
        value = INT_VAL(atoll(parser.previous.start));
    } else if (parser.previous.type == TOKEN_NUMBER) {
        value = FLOAT_VAL(strtod(parser.previous.start, NULL));
    } else {
        __CLOX_ERROR("Unsupported number type!");
    }
    emit_constant(value);
}

void unary(bool can_assign) {
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

void _and(bool can_assign) {
    int end_jump = emit_jump(OP_JUMP_IF_FALSE);

    emit_byte(OP_POP);
    parse_precedence(PREC_AND);
    patch_jump(end_jump);
}

void _or(bool can_assign) {
    int else_jump = emit_jump(OP_JUMP_IF_FALSE);
    int end_jump = emit_jump(OP_JUMP);
    patch_jump(else_jump);

    emit_byte(OP_POP);
    parse_precedence(PREC_OR);
    patch_jump(end_jump);
}

void binary(bool can_assign) {
    tokentype_t operator_type = parser.previous.type;
    parse_rule_t* rule = get_rule(operator_type);
    parse_precedence((precedence_t)(rule->precedence + 1));

    switch(operator_type) {
        case TOKEN_BANG_EQUAL:    emit_byte_2(OP_EQUAL, OP_NOT);   break;
        case TOKEN_EQUAL_EQUAL:   emit_byte  (OP_EQUAL);           break;
        case TOKEN_GREATER:       emit_byte  (OP_GREATER);         break;
        case TOKEN_GREATER_EQUAL: emit_byte_2(OP_LESS, OP_NOT);    break;
        case TOKEN_LESS:          emit_byte  (OP_LESS);            break;
        case TOKEN_LESS_EQUAL:    emit_byte_2(OP_GREATER, OP_NOT); break;

        case TOKEN_PLUS:          emit_byte  (OP_ADD);             break;
        case TOKEN_MINUS:         emit_byte  (OP_SUBTRACT);       break;
        case TOKEN_STAR:          emit_byte  (OP_MULTIPLY);        break;
        case TOKEN_SLASH:         emit_byte  (OP_DIVIDE);          break;
        case TOKEN_FLOOR_DIVIDE:  emit_byte  (OP_FLOOR_DIVIDE);    break;
        case TOKEN_PERCENT:       emit_byte  (OP_MOD);             break;

        case TOKEN_AMPERSAND:     emit_byte  (OP_BIT_AND);         break;
        case TOKEN_PIPE:          emit_byte  (OP_BIT_OR);          break;
        case TOKEN_CAP:           emit_byte  (OP_BIT_XOR);         break;

        case TOKEN_LEFT_SHIFT:    emit_byte  (OP_LEFT_SHIFT);      break;
        case TOKEN_RIGHT_SHIFT:   emit_byte  (OP_RIGHT_SHIFT);     break;
        default: return;
    }
}

void literal(bool can_assign) {
    tokentype_t token_type = parser.previous.type;
    switch(token_type) {
        case TOKEN_NIL:           emit_byte  (OP_NIL); break;
        case TOKEN_TRUE:          emit_byte  (OP_TRUE); break;
        case TOKEN_FALSE:         emit_byte  (OP_FALSE); break;
        default: return;
    }
}

void lambda(bool can_assign) {
    compile_lambda(TYPE_FUNCTION);
}

void string(bool can_assign) {
    emit_constant(OBJECT_VAL(copy_string(parser.previous.start + 1,
                                parser.previous.length - 2)));
}

void variable(bool can_assign) {
    named_variable(parser.previous, can_assign);
}

static uint8_t argument_list() {
    uint8_t arg_count = 0;
    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            expression();
            if (arg_count == 255) {
                __CLOX_COMPILER_PREVIOUS_ERROR("can't have more then 255 arguments.");
            }
            arg_count++;
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    return arg_count;
}

void call(bool can_assign) {
    uint8_t arg_count = argument_list();
    emit_byte_2(OP_CALL, arg_count);
}

void dot(bool can_assign) {
    consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint16_t name = identifier_constant(&parser.previous);

    if (can_assign && match(TOKEN_EQUAL)) {
        expression();
        if (name <= __OP_CONSTANT_MAX_INDEX) {
            emit_byte_2(OP_SET_PROPERTY, name & __UINT8_MASK);
        } else {
            uint8_t hi = (name >> 8) & __UINT8_MASK;
            uint8_t lo = (name     ) & __UINT8_MASK;
            emit_byte(OP_SET_PROPERTY_LONG);
            emit_byte_2(hi, lo);
        }
    } else {
        if (name <= __OP_CONSTANT_MAX_INDEX) {
            emit_byte_2(OP_GET_PROPERTY, name & __UINT8_MASK);
        } else {
            uint8_t hi = (name >> 8) & __UINT8_MASK;
            uint8_t lo = (name     ) & __UINT8_MASK;
            emit_byte(OP_GET_PROPERTY_LONG);
            emit_byte_2(hi, lo);
        }
    }
}

object_function_t* compile(const char* source) {
    /*
     *  Collect garbage before compiling, and then
     *  pause garbage collector while the vm is compiling
     *
     *  When compiling, there shouldn't be any objects
     *  being freed
     */
    collect_garbage();
    bool enclosed_gc_setting = do_garbage_collector;
    do_garbage_collector = false;

    init_scanner(source);

    compiler_t compiler;
    init_compiler(&compiler, TYPE_SCRIPT);
    compiling_chunk = &compiler.function->chunk;

    parser.had_error = false;
    parser.panic_mode = false;

    advance();

    while (!match(TOKEN_EOF)) {
        declaration();
    }

    object_function_t* func = end_compiler();
    merge_temporary();

    do_garbage_collector = enclosed_gc_setting;
    return parser.had_error ? NULL : func;
}

void mark_compiler_roots() {
    compiler_t *compiler = current;
    while (compiler) {
        mark_object((object_t*)compiler->function);
        compiler = compiler->enclosing;
    }
}