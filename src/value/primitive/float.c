
#include "value/value.h"
#include "value/primitive/float.h"

#define V double
#define AS_V(value) AS_NUMBER(value)
#define V_VAL(value) FLOAT_VAL(value)

value_t __float_add(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va + vb);
}

value_t __float_sub(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va - vb);
}

value_t __float_mul(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va * vb);
}

value_t __float_div(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va / vb);
}

#undef V_VAL
#undef AS_V
#undef V