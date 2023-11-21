#ifndef CLOX_VALUE_PRIMITIVE_FLOAT_H_
#define CLOX_VALUE_PRIMITIVE_FLOAT_H_

#include "common.h"

#include "value/value.h"

value_t __float_add(value_t a, value_t b);
value_t __float_sub(value_t a, value_t b);
value_t __float_mul(value_t a, value_t b);
value_t __float_div(value_t a, value_t b);


#endif