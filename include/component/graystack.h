//
// Created by shenshuhan on 12/25/23.
//

#ifndef CLOX_GRAYSTACK_H
#define CLOX_GRAYSTACK_H

#include "utils/stack.h"
#include "value/value.h"


void push_gray_stack(clox_stack_t* stack, object_t* obj);
object_t* pop_gray_stack(clox_stack_t* stack);

#endif //CLOX_GRAYSTACK_H
