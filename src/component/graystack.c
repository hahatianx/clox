//
// Created by shenshuhan on 12/25/23.
//

#include "component/graystack.h"

void push_gray_stack(clox_stack_t* stack, object_t* obj) {
    push_stack(stack, (void*)obj);
}

object_t* pop_gray_stack(clox_stack_t* stack) {
    return (object_t*)pop_stack(stack);
}

