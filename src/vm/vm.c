#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "common.h"
#include "constant.h"

#include "debug/debug.h"

#include "component/vartable.h"
#include "component/valuetable.h"

#include "value/value.h"
#include "value/primitive/float.h"
#include "value/primitive/integer.h"

#include "vm/vm.h"
#include "vm/compiler.h"

vm_t vm;

static void reset_stack() {
    vm.stack_top = vm.stack;
}

static void runtime_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    reset_stack();
}

static value_t peek(int distance) {
    return vm.stack_top[-1 - distance];
}

static bool is_falsey(value_t value) {
    return IS_NIL(value) || 
        (IS_BOOL(value) && !AS_BOOL(value)) || 
        (IS_INT(value) && !AS_INT(value));
}

static void free_objects() {
    while(!list_empty(&vm.obj)) {
        list_remove_head(&vm.obj);
    }
}

static interpret_result_t run() {
#define READ_BYTE() (*vm.ip++)
#define READ_SHORT() (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8) | vm.ip[-1]))
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_CONSTANT_LONG() (vm.chunk->constants.values[READ_SHORT()])
#define READ_STRING() (AS_STRING(READ_CONSTANT()))
#define READ_STRING_LONG()  (AS_STRING(READ_CONSTANT_LONG()))
#define BINARY_OP(value_type, op) \
    do {  \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtime_error("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop());   \
        double a = AS_NUMBER(pop());   \
        push(value_type(a op b));    \
    } while (0)
#define ADD_OP \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtime_error("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        value_t b = pop(); \
        value_t a = pop(); \
        if (!IS_INT(a) || !IS_INT(b)) { \
            push(__float_add(a, b)); \
        } else { \
            push(__integer_add(a, b)); \
        } \
    } while (0)
#define SUB_OP \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtime_error("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        value_t b = pop(); \
        value_t a = pop(); \
        if (!IS_INT(a) || !IS_INT(b)) { \
            push(__float_sub(a, b)); \
        } else { \
            push(__integer_sub(a, b)); \
        } \
    } while (0)
#define MUL_OP \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtime_error("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        value_t b = pop(); \
        value_t a = pop(); \
        if (!IS_INT(a) || !IS_INT(b)) { \
            push(__float_mul(a, b)); \
        } else { \
            push(__integer_mul(a, b)); \
        } \
    } while (0)
#define DIV_OP \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtime_error("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        value_t b = pop(); \
        value_t a = pop(); \
        if (fabs(AS_NUMBER(b)) < __FLOAT_PRECISION) { \
            runtime_error("Divisor cannot be zero."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        push(__float_div(a, b)); \
    } while (0)
#define INTEGER_BINARY_OP(op_method) \
    do { \
        if (!IS_INT(peek(0)) || !IS_INT(peek(1))) { \
            runtime_error("Operands must be integers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        value_t b = pop(); \
        value_t a = pop(); \
        push( op_method (a, b)); \
    } while (0)



    for(;;) {

#ifdef DEBUG_TRACE_EXECUTION
    int printed = 0;
    for (value_t* slot = vm.stack; slot < vm.stack_top; slot++) {
        printf("[ ");
        printed += print_value(*slot) + 4;
        printf(" ]");
    }
    for (int i = printed; i < 50; ++i) printf(" ");
    disassemble_instruction(vm.chunk,
        (int)(vm.ip - vm.chunk->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                value_t constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_CONSTANT_LONG: {
                value_t constant = READ_CONSTANT_LONG();
                push(constant);
                break;
            }
            case OP_GREATER:       BINARY_OP(BOOL_VAL,   >);           break;
            case OP_LESS:          BINARY_OP(BOOL_VAL,   <);           break;
            case OP_ADD:           ADD_OP;                             break;
            case OP_SUBSTRACT:     SUB_OP;                             break;
            case OP_MULTIPLY:      MUL_OP;                             break;
            case OP_DIVIDE:        DIV_OP;                             break;
            case OP_FLOOR_DIVIDE:  INTEGER_BINARY_OP(__integer_div);   break;
            case OP_BIT_AND:       INTEGER_BINARY_OP(__integer_and);   break;
            case OP_BIT_XOR:       INTEGER_BINARY_OP(__integer_xor);   break;
            case OP_BIT_OR:        INTEGER_BINARY_OP(__integer_or);    break;
            case OP_MOD:           INTEGER_BINARY_OP(__integer_mod);   break;
            case OP_LEFT_SHIFT:    INTEGER_BINARY_OP(__integer_lsh);   break;
            case OP_RIGHT_SHIFT:   INTEGER_BINARY_OP(__integer_rsh);   break;
            case OP_EQUAL: {
                value_t a = pop();
                value_t b = pop();
                push(BOOL_VAL(values_equal(a, b)));
                break;
            }
            case OP_NOT:
                push(BOOL_VAL(is_falsey(pop())));
                break;
            case OP_NEGATE:     
                if (!IS_NUMBER(peek(0))) {
                    runtime_error("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (IS_INT(peek(0))) {
                    push(INT_VAL(-AS_INT(pop())));
                } else {
                    push(FLOAT_VAL(-AS_FLOAT(pop())));
                }
                break;
            case OP_RETURN: {
                /*
                  Be cautious. pop() here may result in stack underflow
                */
                // print_value(pop());
                printf("\n");
                return INTERPRET_OK;
            }
            case OP_NIL:   push(NIL_VAL); break;
            case OP_TRUE:  push(BOOL_VAL(1)); break;
            case OP_FALSE: push(BOOL_VAL(0)); break;
            case OP_PRINT: {
                print_value(pop());
                break;
            }
            case OP_PRINTLN: {
                print_value(pop());
                printf("\n");
                break;
            }
            case OP_POP: {
                pop();
                break;
            }
            case OP_POPN: {
                uint8_t stacks_to_pop = READ_BYTE();
                vm.stack_top -= stacks_to_pop;
                break;
            }
            case OP_DEFINE_GLOBAL: {
                object_string_t* name = READ_STRING();
                table_set_var(&vm.globals, name, (var_t) {false, peek(0)});
                pop();
                break;
            }
            case OP_DEFINE_GLOBAL_LONG: {
                object_string_t* name = READ_STRING_LONG();
                table_set_var(&vm.globals, name, (var_t) {false, peek(0)});
                pop();
                break;
            }
            case OP_DEFINE_MUT_GLOBAL: {
                object_string_t* name = READ_STRING();
                table_set_var(&vm.globals, name, (var_t) {true , peek(0)});
                pop();
                break;
            }
            case OP_DEFINE_MUT_GLOBAL_LONG: {
                object_string_t* name = READ_STRING_LONG();
                table_set_var(&vm.globals, name, (var_t) {true , peek(0)});
                pop();
                break;
            }
            case OP_GET_GLOBAL: {
                object_string_t* name = READ_STRING();
                var_t var;
                if (!table_get_var(&vm.globals, name, &var)) {
                    __CLOX_RUNTIME_ERROR("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(var.v);
                break;
            }
            case OP_GET_GLOBAL_LONG: {
                object_string_t* name = READ_STRING_LONG();
                var_t var;
                if (!table_get_var(&vm.globals, name, &var)) {
                    __CLOX_RUNTIME_ERROR("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(var.v);
                break;
            }
            case OP_SET_GLOBAL: {
                object_string_t* name = READ_STRING();
                var_t var;
                if (!table_get_var(&vm.globals, name, &var)) {
                    __CLOX_RUNTIME_ERROR("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                } else {
                    if (var.mutable) {
                        table_set_var(&vm.globals, name, (var_t) {true, peek(0)});
                    } else {
                        __CLOX_RUNTIME_ERROR("Cannot assign new values to immutable variable '%s'.", name->chars);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                }
                break;
            }
            case OP_SET_GLOBAL_LONG: {
                object_string_t* name = READ_STRING_LONG();
                var_t var;
                if (!table_get_var(&vm.globals, name, &var)) {
                    __CLOX_RUNTIME_ERROR("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                } else {
                    if (var.mutable) {
                        table_set_var(&vm.globals, name, (var_t) {true, peek(0)});
                    } else {
                        __CLOX_RUNTIME_ERROR("Cannot assign new values to immutable variable '%s'.", name->chars);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                }
                break;
            }
            case OP_DEFINE_LOCAL: {
                uint8_t arg = ((void*)vm.stack_top - (void*)vm.stack) / sizeof(value_t) - 1;
                vm.local[arg].mutable = false;
                break;
            }
            case OP_DEFINE_MUT_LOCAL: {
                uint8_t arg = ((void*)vm.stack_top - (void*)vm.stack) / sizeof(value_t) - 1;
                vm.local[arg].mutable = true;
                break;
            }
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(vm.stack[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                if (!vm.local[slot].mutable) {
                    __CLOX_RUNTIME_ERROR("Cannot assign new values to immutable variable.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                vm.stack[slot] = peek(0);
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                vm.ip += is_falsey(peek(0)) * offset;
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                vm.ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                vm.ip -= offset;
                break;
            }
            default:
                __CLOX_ERROR("The clox virtual does not support this bypte code operation.");
        }
    }

#undef INTEGER_BINARY_OP
#undef DIV_OP
#undef MUL_OP
#undef SUB_OP
#undef ADD_OP
#undef BINARY_OP
#undef READ_STRING_LONG
#undef READ_STRING
#undef READ_CONSTANT_LONG
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_BYTE
}

void init_vm() {
    reset_stack();
    list_init(&vm.obj);
    init_table(&vm.globals);
    init_table(&vm.strings);
}


void free_vm() {
    free_table(&vm.strings);
    free_table_var(&vm.globals);
    free_objects();
}

interpret_result_t interpret(const char* source) {
    chunk_t chunk;
    init_chunk(&chunk);

    if (!compile(source, &chunk)) {
        free_chunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;


    interpret_result_t result = run();
    free_chunk(&chunk);

    return result;
}

void push(value_t value) {
    *vm.stack_top = value;
    vm.stack_top++;
#ifdef DEBUG_VM_EXECUTION
    printf("Current stack pointer %lu\n", vm.stack_top - vm.stack);
#endif
}

value_t pop() {
#ifdef DEBUG_VM_EXECUTION
    printf("Current stack pointer %lu\n", vm.stack_top - vm.stack);
#endif
    vm.stack_top--;
    return *vm.stack_top;
}