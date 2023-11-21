//
// Created by shenshuhan on 11/20/23.
//

#include <time.h>

#include "value/native/clock.h"

value_t clock_native(int argc, value_t* args) {
    return FLOAT_VAL((double)clock() / CLOCKS_PER_SEC);
}
