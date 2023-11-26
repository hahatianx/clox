//
// Created by shenshuhan on 11/20/23.
//

#include <time.h>

#include "value/native/clock.h"

value_t clock_native(__attribute__((unused)) int argc, __attribute__((unused)) value_t* args) {
    return FLOAT_VAL((double)clock() / CLOCKS_PER_SEC);
}
