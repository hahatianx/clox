
#include "constant.h"

#include "basic/chunk.h"
#include "basic/memory.h"


void init_chunk(chunk_t* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    init_value_array(&chunk->constants);
}

void write_chunk(chunk_t* chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->count + 1) {
        int old_capacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(old_capacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, old_capacity, chunk->capacity);
    }
    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count ++;
}

void free_chunk(chunk_t* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    free_value_array(&chunk->constants);
    init_chunk(chunk);
}


int add_constant(chunk_t* chunk, value_t value) {
    write_value_array(&chunk->constants, value);
    return chunk->constants.count - 1;
}

int write_constant(chunk_t* chunk, value_t value, int line) {
    int index = add_constant(chunk, value);
    if (index > __OP_CONSTANT_LONG_MAX_INDEX)
        return 1;
    if (index <= __OP_CONSTANT_MAX_INDEX) {
        write_chunk(chunk, OP_CONSTANT, line);
        write_chunk(chunk, index, line);
    } else {
        write_chunk(chunk, OP_CONSTANT_LONG, line);
        write_chunk(chunk, (index >> 8) & __UINT8_MASK, line);
        write_chunk(chunk, (index     ) & __UINT8_MASK, line);
    }
    return 0;
}

int get_line(chunk_t* chunk, int offset) {
    return chunk->lines[offset];
}