#ifndef __CLOX_DEBUG_H__
#define __CLOX_DEBUG_H__

#include "basic/chunk.h"

void disassemble_chunk(chunk_t* chunk, const char* name);
int disassemble_instruction(chunk_t* chunk, int offset);


#endif