#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "constant.h"
#include "switch.h"

#include "component/vartable.h"
#include "component/valuetable.h"
#include "component/graystack.h"

#include "vm/runtime.h"
#include "vm/compiler.h"

#ifdef DEBUG_PRINT_CODE
#include "debug/debug.h"
#include "switch.h"
#include "component/valuetable.h"

#endif

vm_t vm;

#ifdef DEBUG_VM_MEMORY
static void print_vm_structure() {
    printf("----------------------------------\n");
    // print var tables
    for (int i = 0; i < vm.globals.capacity; ++i) {
        table_entry_t *entry = &vm.globals.entries[i];
        if (entry->key == NULL) continue;
        printf("table item %p, key ", entry);
        print_value(OBJECT_VAL(entry->key));
        printf(", value ");
        if (entry->value == NULL) printf("null\n");
        else {
            print_value(((var_t *) entry->value)->v);
            printf(" mutable? %d", ((var_t *) entry->value)->mutable);
            printf("\n");
        }
    }

    // print interned strings
#ifdef DEBUG_PRINT_STRINGS
    printf("\nstrings\n");
    for (int i = 0; i < vm.strings.capacity; ++i) {
        table_entry_t *entry = &vm.strings.entries[i];
        if (entry->key == NULL) continue;
        print_value(OBJECT_VAL(entry->key));
        printf("\n");
    }
    printf("\n");
#endif

    // print objects
#ifdef DEBUG_PRINT_OBJCTS
    object_t* iter = NULL;
    list_iterate_begin(object_t, link, &vm.obj, iter) {
        printf("object %p, type ", iter);
        switch (iter->type) {
            case OBJ_STRING:
                printf("%10s", "STRING");
                break;
            case OBJ_FUNCTION:
                printf("%10s", "FUNCTION");
                break;
            case OBJ_NATIVE:
                printf("%10s", "NATIVE");
                break;
            case OBJ_CLOSURE:
                printf("%10s", "CLOSURE");
                break;
            case OBJ_UPVALUE:
                printf("%10s", "UPVALUE");
                break;
        }
        printf(", value ");
        print_value(OBJECT_VAL(iter));
        printf("\n");
    } list_iterate_end();
    printf("----------------------------------\n\n\n");
#endif
}
#endif

static void reset_stack() {
    vm.stack_top = vm.stack;
    vm.frame_count = 0;
    list_init(&vm.open_upvalues);
}

static void runtime_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frame_count - 1; i >= 0; --i) {
        callframe_t* frame = &vm.frames[i];
        object_function_t* func = frame->closure->function;
        size_t instruction = frame->ip - func->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", func->chunk.lines[instruction]);
        if (func->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", func->name->chars);
        }
    }

    reset_stack();
}

static value_t peek(int distance) {
    return vm.stack_top[-1 - distance];
}

static bool is_falsy(value_t value) {
    return IS_NIL(value) || 
        (IS_BOOL(value) && !AS_BOOL(value)) || 
        (IS_INT(value) && !AS_INT(value));
}

static void free_objects() {
    while(!list_empty(&vm.obj)) {
        list_remove_head(&vm.obj);
    }
}

static bool call(object_closure_t * closure, int arg_count) {
    if (arg_count != closure->function->arity) {
        __CLOX_RUNTIME_ERROR("Expected %d arguments but got %d.",
            closure->function->arity, arg_count);
        return false;
    }

    if (vm.frame_count == FRAMES_MAX) {
        __CLOX_RUNTIME_ERROR("Stack overflow.");
        return false;
    }

    callframe_t* frame = &vm.frames[vm.frame_count++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    int bias = ((vm.stack_top - arg_count - 1) - vm.stack);
    frame->slots = vm.stack_top - arg_count - 1;
    frame->local_meta = &vm.local[bias];
    return true;
}

static object_upvalue_t* capture_upvalue(value_t* local, var_metadata_t* local_meta) {
    object_upvalue_t* iter = NULL;
    list_iterate_begin(object_upvalue_t, link, &vm.open_upvalues, iter) {
        if (iter->location < local)
            goto STOP_LOOP;
        if (iter->location == local)
            return iter;
    } list_iterate_end();
    STOP_LOOP:;

    object_upvalue_t* created_upvalue = new_upvalue(local);
    created_upvalue->mutable = local_meta->mutable;
    if (iter == NULL)
        list_insert_head(&vm.open_upvalues, &created_upvalue->link);
    else
        list_insert_after(iter->link.l_prev, &created_upvalue->link);
    return created_upvalue;
}

static void close_upvalues(value_t* last) {
    while (!list_empty(&vm.open_upvalues)) {
        object_upvalue_t* iter = list_head_item(object_upvalue_t, link, &vm.open_upvalues);
        if (iter->location < last) break;
        iter->closed = *iter->location;
        iter->location = &iter->closed;
        list_remove_head(&vm.open_upvalues);
    }
}

static bool call_value(value_t callee, int arg_count) {
    if (IS_OBJECT(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_BOUND_METHOD: {
                object_bound_method_t *bound = AS_BOUND_METHOD(callee);
                vm.stack_top[-arg_count - 1] = bound->receiver;
                return call(bound->method, arg_count);
            }
            case OBJ_CLASS: {
                object_class_t *klass = AS_CLASS(callee);
                vm.stack_top[-arg_count - 1] = OBJECT_VAL(new_instance(klass));
                value_t init;

                if (table_get_value(&klass->methods, vm.init_string, &init)) {
                    return call(AS_CLOSURE(init), arg_count);
                }

                return true;
            }
            case OBJ_CLOSURE:
                return call(AS_CLOSURE(callee), arg_count);
//            case OBJ_FUNCTION:
//                return call(AS_FUNCTION(callee), arg_count);
            case OBJ_NATIVE: {
                object_native_func_t* native = (object_native_func_t*) AS_OBJECT(callee);
                int arity = native->arity;
                if (arg_count != arity) {
                    __CLOX_RUNTIME_ERROR("Expect %d arguments, but found %d.", arity, arg_count);
                    return false;
                }
                native_fn_t f = native->function;
                value_t result = f(arg_count, vm.stack_top - arg_count);
                vm.stack_top -= arg_count + 1;
                push(result);
                return true;
            }
            default:
                break;
        }
    }
    __CLOX_RUNTIME_ERROR("The object is not callable.");
    return false;
}

static bool invoke_from_class(object_class_t *klass, object_string_t *name, int arg_count) {
    value_t method;
    if(!table_get_value(&klass->methods, name, &method)) {
        runtime_error("Undefined property '%s'.", name->chars);
        return false;
    }
    return call(AS_CLOSURE(method), arg_count);
}

static bool invoke(object_string_t* name, int arg_count) {
    value_t receiver = peek(arg_count);

    if (!IS_INSTANCE(receiver)) {
        runtime_error("Only instances have methods.");
        return false;
    }

    object_instance_t *instance = AS_INSTANCE(receiver);

    value_t value;
    if (table_get_value(&instance->fields, name, &value)) {
        vm.stack_top[-arg_count - 1] = value;
        return call_value(value, arg_count);
    }

    return invoke_from_class(instance->klass, name, arg_count);
}

static void define_method(object_string_t *name) {
    value_t method = peek(0);
    object_class_t *klass = AS_CLASS(peek(1));
    table_set_value(&klass->methods, name, method);
    pop();
}

static bool bind_method(object_class_t *klass, object_string_t *name) {
    value_t method;
    if (!table_get_value(&klass->methods, name, &method)) {
        runtime_error("Undefined property '%s'.", name->chars);
        return false;
    }

    object_bound_method_t *bound = new_bound_method(peek(0), AS_CLOSURE(method));
    pop();
    push(OBJECT_VAL(bound));
    return true;
}

static interpret_result_t run() {

    callframe_t* frame = &vm.frames[vm.frame_count - 1];
    register uint8_t *ip = frame->ip;

#define CONTEXT_SWITCH(to)        \
    do {                          \
        frame->ip = ip;           \
        ip = (to)->ip;            \
        frame = (to);             \
    } while(0)

#define READ_BYTE() (*ip++)
#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_CONSTANT_LONG() (frame->closure->function->chunk.constants.values[READ_SHORT()])
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
        printed += printf(" %p ", slot);
        printed += print_value(*slot) + 4;
        printf(" ]");
    }
    for (int i = printed; i < 150; ++i) printf(" ");
    disassemble_instruction(&frame->closure->function->chunk,
        (int)(ip - frame->closure->function->chunk.code));
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
            case OP_SUBTRACT:      SUB_OP;                             break;
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
                push(BOOL_VAL(is_falsy(pop())));
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
                    pop() here pops the callframe_t on stack
                */
                value_t value = pop();
                close_upvalues(frame->slots);
                vm.frame_count--;
                if (vm.frame_count == 0) {
                    pop();
                    return INTERPRET_OK;
                }
                vm.stack_top = frame->slots;
                push(value);
                CONTEXT_SWITCH(&vm.frames[vm.frame_count - 1]);
                break;
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
                uint8_t arg = vm.stack_top - frame->slots - 1;
                frame->local_meta[arg].mutable = false;
                break;
            }
            case OP_DEFINE_MUT_LOCAL: {
                uint8_t arg = vm.stack_top - frame->slots - 1;
                frame->local_meta[arg].mutable = true;
                break;
            }
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                if (!frame->local_meta[slot].mutable) {
                    __CLOX_RUNTIME_ERROR("Cannot assign new values to immutable variable.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame->slots[slot] = peek(0);
                break;
            }
            case OP_CLASS: {
                push(OBJECT_VAL(new_class(READ_STRING())));
                break;
            }
            case OP_CLASS_LONG: {
                push(OBJECT_VAL(new_class(READ_STRING_LONG())));
                break;
            }
            case OP_INHERIT: {
                value_t superclass = peek(1);
                object_class_t *subclass = AS_CLASS(peek(0));
                if (!IS_CLASS(superclass)) {
                    runtime_error("Superclass must be a class.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                table_add_all(&AS_CLASS(superclass)->methods, &subclass->methods);
                pop();
                break;
            }
            case OP_METHOD:
                define_method(READ_STRING());
                break;
            case OP_METHOD_LONG:
                define_method(READ_STRING_LONG());
                break;
            case OP_GET_SUPER:
            case OP_GET_SUPER_LONG: {
                object_string_t *name = NULL;
                if (instruction == OP_GET_SUPER)
                    name = READ_STRING();
                else
                    name = READ_STRING_LONG();
                object_class_t *superclass = AS_CLASS(pop());

                if (!bind_method(superclass, name))
                    return INTERPRET_RUNTIME_ERROR;
                break;
            }
            case OP_GET_PROPERTY:
            case OP_GET_PROPERTY_LONG: {
                if (!IS_INSTANCE(peek(0))) {
                    runtime_error("Only instances have properties.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                object_string_t *name;
                if (instruction == OP_GET_PROPERTY)
                    name = READ_STRING();
                else
                    name = READ_STRING_LONG();

                object_instance_t *instance = AS_INSTANCE(peek(0));
                value_t value;
                if (table_get_value(&instance->fields, name, &value)) {
                    pop();
                    push(value);
                    break;
                }

                if (!bind_method(instance->klass, name))
                    return INTERPRET_RUNTIME_ERROR;
                break;
            }
            case OP_SET_PROPERTY:
            case OP_SET_PROPERTY_LONG: {
                if (!IS_INSTANCE(peek(1))) {
                    runtime_error("Only instances have properties.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                object_string_t *name;
                if (instruction == OP_SET_PROPERTY)
                    name = READ_STRING();
                else
                    name = READ_STRING_LONG();

                object_instance_t *instance = AS_INSTANCE(peek(1));
                table_set_value(&instance->fields, name, peek(0));

                value_t value = pop();
                pop(); // pop instance
                push(value);
                break;
            }
            case OP_GET_ARRAY_INDEX: {
                if (!IS_LIST(peek(1))) {
                    runtime_error("Only arrays have indices.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (!IS_INT(peek(0))) {
                    runtime_error("Only integers can be array indices.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                value_t index = pop();
                value_t array = pop();
                value_t value;
                int result = get_list_value(AS_LIST(array), AS_INT(index), &value);
                if (result) {
                    runtime_error("Array index out of bound.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_SET_ARRAY_INDEX: {
                if (!IS_LIST(peek(2))) {
                    runtime_error("Only arrays have indices.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (!IS_INT(peek(1))) {
                    runtime_error("Only integers can be array indices.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                value_t value = pop();
                value_t index = pop();
                value_t array = pop();
                int result = set_list_value(AS_LIST(array), AS_INT(index), value);
                if (result) {
                    runtime_error("Array index out of bound.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_ARRAY: {
                value_t init = pop();
                value_t length = pop();
                object_list_t *list = new_list(AS_INT(length), init);
                push(OBJECT_VAL(list));
                break;
            }
            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push(*frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                if (!frame->closure->upvalues[slot]->mutable) {
                    __CLOX_RUNTIME_ERROR("Cannot assign new values to immutable variable.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                *frame->closure->upvalues[slot]->location = peek(0);
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                ip += is_falsy(peek(0)) * offset;
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                ip -= offset;
                break;
            }
            case OP_CALL: {
                int arg_count = READ_BYTE();
                if (!call_value(peek(arg_count), arg_count)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                CONTEXT_SWITCH(&vm.frames[vm.frame_count - 1]);
                break;
            }
            case OP_INVOKE:
            case OP_INVOKE_LONG: {
                object_string_t *method = NULL;
                if (instruction == OP_INVOKE)
                    method = READ_STRING();
                else
                    method = READ_STRING_LONG();
                int arg_count = READ_BYTE();
                if (!invoke(method, arg_count)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                CONTEXT_SWITCH(&vm.frames[vm.frame_count - 1]);
                break;
            }
            case OP_CLOSURE: {
                // TODO: READ_CONSTANT is not accurate here
                object_function_t *func = AS_FUNCTION(READ_CONSTANT());
                object_closure_t *closure = new_closure(func);
                for (int i = 0; i < closure->upvalue_count; i++) {
                    uint8_t is_local = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if (is_local) {
                        closure->upvalues[i] = capture_upvalue(
                                frame->slots + index, frame->local_meta + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                push(OBJECT_VAL(closure));
                break;
            }
            case OP_CLOSURE_UPVALUE: {
                close_upvalues(vm.stack_top - 1);
                pop();
                break;
            }
            default:
                printf("OP: %d\n", instruction);
                __CLOX_ERROR("The clox virtual machine does not support this byte code operation.");
        }

        /*
         *  All objects are added to temporary list while executing the operations.
         *  They are merged into main vm obj list once the operations are fully completed.
         */
        merge_temporary();

#ifdef DEBUG_VM_MEMORY
        print_vm_structure();
#endif
    } // end for

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
#undef CONTEXT_SWITCH
}

static void define_native(const char* name, int argc, native_fn_t func) {
    push(OBJECT_VAL(copy_string(name, (int)strlen(name))));
    push(OBJECT_VAL(new_native(argc, func)));
    table_set_var(&vm.globals, AS_STRING(vm.stack[0]), (var_t) {false, vm.stack[1]} );
    pop();
    pop();
}

void init_vm() {
    /*
     * Initialize global vm parameters
     */
    do_garbage_collector = true;

    reset_stack();
    list_init(&temporary_objs);
    list_init(&vm.obj);
    list_init(&vm.open_upvalues);
    init_table(&vm.globals);
    init_table(&vm.strings);
    init_stack(&vm.gray_stack);

    vm.init_string = NULL;
    vm.init_string = copy_string("init", 4);

    define_native("clock", 0, clock_native);
    define_native("type", 1, type_native);
}

void free_vm() {
    free_table(&vm.strings);
    free_table_var(&vm.globals);
    free_stack(&vm.gray_stack);
    vm.init_string = NULL;
    free_objects();
}

interpret_result_t interpret(const char* source) {
    object_function_t* func = compile(source);
    if (func == NULL)
        return INTERPRET_COMPILE_ERROR;

    push(OBJECT_VAL(func));
    object_closure_t *closure = new_closure(func);
    pop();
    push(OBJECT_VAL(closure));
    call(closure, 0);

    merge_temporary();
    interpret_result_t result = run();
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

void remove_unused_strings() {
    table_t *table = &vm.strings;
    for (int i = 0; i < table->capacity; ++i) {
        table_entry_t *entry = &table->entries[i];
        if (entry->key && !entry->key->obj.is_marked) {
#ifdef DEBUG_LOG_GC
            printf("Remove %s from interned strings.\n", ((object_string_t*)entry->key)->chars);
#endif
            table_delete(table, entry->key, NULL);
        }
    }
}
