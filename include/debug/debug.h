#ifndef CLOX_DEBUG_H_
#define CLOX_DEBUG_H_

#include "constant.h"

#include "vm/compiler.h"
#include "basic/chunk.h"

void disassemble_chunk(chunk_t* chunk, const char* name);
int disassemble_instruction(chunk_t* chunk, int offset);

void disassemble_locals(compiler_t* compiler, const char* name);
void disassemble_local_var(int slot, local_t* local);


#endif