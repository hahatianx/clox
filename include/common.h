#ifndef CLOX_COMMON_H_
#define CLOX_COMMON_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// #define DEBUG_VM_EXECUTION
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION
// #define DEBUG_MEMORY_BLOCK
// #define DEBUG_TOKEN_SCANNED

#define __offset(type, member) ((uint64_t)((char*)&((type*)NULL)->member))
#define __object(type, pointer, member) ((type*)((char*)(pointer) - __offset(type, member)))

#endif