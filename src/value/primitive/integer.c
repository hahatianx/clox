
#include "value/value.h"
#include "value/primitive/integer.h"

#define V int64_t
#define AS_V(value) AS_INT(value)
#define V_VAL(value) INT_VAL(value)

value_t __integer_add(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va + vb);
}

value_t __integer_sub(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va - vb);
}

value_t __integer_mul(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va * vb);
}

value_t __integer_div(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va / vb);
}

value_t __integer_mod(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va % vb);
}

value_t __integer_lsh(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va << vb);
}

value_t __integer_rsh(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va >> vb);
}

value_t __integer_and(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va & vb);
}

value_t __integer_or(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va | vb);
}

value_t __integer_xor(value_t a, value_t b) {
    V va = AS_V(a);
    V vb = AS_V(b);
    return V_VAL(va ^ vb);
}

#undef V
#undef AS_V
#undef V_VAL