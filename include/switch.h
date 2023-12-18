//
// Created by shenshuhan on 12/27/23.
//

#ifndef CLOX_SWITCH_H
#define CLOX_SWITCH_H

#include "common.h"

/* new objects are created while doing compiling, but these objects can be wiped out by
 * following reallocate
 *
 * while compiling, the vm should block garbage collector
 */
extern bool do_garbage_collector;

#endif //CLOX_SWITCH_H
