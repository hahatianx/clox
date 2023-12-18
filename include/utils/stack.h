//
// Created by shenshuhan on 12/25/23.
//

#ifndef CLOX_STACK_H
#define CLOX_STACK_H

typedef struct clox_stack {
    int capacity;
    int count;
    void** stack;
} clox_stack_t;

void init_stack(clox_stack_t* stack);
void free_stack(clox_stack_t* stack);
void push_stack(clox_stack_t* stack, void* obj);
void* pop_stack(clox_stack_t* stack);

#endif //CLOX_STACK_H
