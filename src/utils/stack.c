//
// Created by shenshuhan on 12/25/23.
//

#include "common.h"
#include "utils/stack.h"
#include "basic/memory.h"

void init_stack(clox_stack_t* stack) {
    stack->capacity = 0;
    stack->count = 0;
    stack->stack = NULL;
}

void free_stack(clox_stack_t* stack) {
    free(stack->stack);
}

void push_stack(clox_stack_t* stack, void* obj) {
    if (stack->capacity < stack->count + 1) {
        stack->capacity = GROW_CAPACITY(stack->capacity);
        stack->stack = (void**)realloc(stack->stack, sizeof(void*) * stack->capacity);
        if (stack->stack == NULL) {
            __CLOX_ERROR("Failed to allocate stack");
        }
    }
    stack->stack[stack->count++] = obj;
}

void* pop_stack(clox_stack_t* stack) {
    return stack->stack[--stack->count];
}