#ifndef __CLOX_ERROR_H__
#define __CLOX_ERROR_H__

#include <stdio.h>

#include "common.h"

#define __CLOX_ERROR(error) \
    do { \
        fprintf(stderr, "Clox error: %s\n", error); \
        exit(1); \
    } while (0)


// error(const char* message) is defined in vm/compiler.c
#define __CLOX_COMPILER_CURRENT_ERROR(err) \
    do { \
        error_at_current(err); \
    } while (0)

#define __CLOX_COMPILER_PREVIOUS_ERROR(err) \
    do { \
        error(err); \
    } while (0)

#define __CLOX_RUNTIME_ERROR runtime_error
    

#endif