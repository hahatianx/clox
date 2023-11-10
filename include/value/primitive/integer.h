#ifndef __CLOX_VALUE_PRIMITIVE_INTEGER_H__
#define __CLOX_VALUE_PRIMITIVE_INTEGER_H__

#include "common.h"

#include "value/value.h"
#include "value/primitive/float.h"

value_t __integer_add(value_t a, value_t b);
value_t __integer_sub(value_t a, value_t b);
value_t __integer_mul(value_t a, value_t b);
value_t __integer_div(value_t a, value_t b);

value_t __integer_mod(value_t a, value_t b);

value_t __integer_lsh(value_t a, value_t b);
value_t __integer_rsh(value_t a, value_t b);
value_t __integer_and(value_t a, value_t b);
value_t __integer_or (value_t a, value_t b);
value_t __integer_xor(value_t a, value_t b);


#endif