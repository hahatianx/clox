#ifndef __CLOX_COMMON_H__
#define __CLOX_COMMON_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION
// #define DEBUG_MEMORY_BLOCK

#define __offset(type, member) ((uint64_t)((char*)&((type*)NULL)->member))
#define __object(type, pointer, member) ((type*)((char*)(pointer) - __offset(type, member)))

#endif