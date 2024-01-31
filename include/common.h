#ifndef CLOX_COMMON_H_
#define CLOX_COMMON_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//#define DEBUG_VM_EXECUTION
//#define DEBUG_VM_MEMORY

#define DEBUG_PRINT_CODE
//#define DEBUG_PRINT_OBJECT
//#define DEBUG_TRACE_EXECUTION
//#define DEBUG_MEMORY_BLOCK
//#define DEBUG_TOKEN_SCANNED

//#define DEBUG_PRINT_TABLE
//#define DEBUG_PRINT_FREED


//#define DEBUG_STRESS_GC
//#define DEBUG_LOG_GC

#define __offset(type, member) ((uint64_t)((char*)&((type*)NULL)->member))
#define __object(type, pointer, member) ((type*)((char*)(pointer) - __offset(type, member)))

#endif