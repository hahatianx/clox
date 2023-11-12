#include <stdio.h>

#include "debug/debug.h"
#include "value/value.h"

static int simple_instruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int constant_instruction(const char* name, chunk_t* chunk, int offset) {
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    print_value(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

static int constant_instruction_long(const char* name, chunk_t* chunk, int offset) {
    uint8_t constant_1 = chunk->code[offset + 1];
    uint8_t constant_2 = chunk->code[offset + 2];
    int constant = (constant_1 << 8) | (constant_2);
    printf("%-16s %4d '", name, constant);
    print_value(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 3;
}

void disassemble_chunk(chunk_t* chunk, const char* name) {
    printf("== %s ==\n", name);
    for (int offset = 0; offset < chunk->count;) {
        offset = disassemble_instruction(chunk, offset);
    }
}

int disassemble_instruction(chunk_t* chunk, int offset) {
    printf("%04d ", offset);

    if (offset > 0 &&
        chunk->lines[offset] == chunk->lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk->lines[offset]);
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset);
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", chunk, offset);
        case OP_CONSTANT_LONG:
            return constant_instruction_long("OP_CONSTANT_LONG", chunk, offset);
        case OP_EQUAL:
            return simple_instruction("OP_EQUAL", offset);
        case OP_GREATER:
            return simple_instruction("OP_GREATER", offset);
        case OP_LESS:
            return simple_instruction("OP_LESS", offset);
        case OP_ADD:
            return simple_instruction("OP_ADD", offset);
        case OP_SUBSTRACT:
            return simple_instruction("OP_SUBSTRACT", offset);
        case OP_MULTIPLY:
            return simple_instruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simple_instruction("OP_DIVIDE", offset);
        case OP_MOD:
            return simple_instruction("OP_MOD", offset);
        case OP_FLOOR_DIVIDE:
            return simple_instruction("OP_FLOOR_DIVIDE", offset);
        case OP_NEGATE:
            return simple_instruction("OP_NEGATE", offset);
        case OP_NIL:
            return simple_instruction("OP_NIL", offset);
        case OP_TRUE:
            return simple_instruction("OP_TRUE", offset);
        case OP_FALSE:
            return simple_instruction("OP_FALSE", offset);
        case OP_PRINT:
            return simple_instruction("OP_PRINT", offset);
        case OP_POP:
            return simple_instruction("OP_POP", offset);
        case OP_DEFINE_GLOBAL:
            return constant_instruction("OP_DEFINE_GLOBAL", chunk, offset);
        case OP_GET_GLOBAL:
            return constant_instruction("OP_GET_GLOBAL", chunk, offset);
        case OP_DEFINE_GLOBAL_LONG:
            return constant_instruction_long("OP_DEFINE_GLOBAL_LONG", chunk, offset);
        case OP_GET_GLOBAL_LONG:
            return constant_instruction_long("OP_GET_GLOBAL_LONG", chunk, offset);
        case OP_BIT_AND:
            return simple_instruction("OP_BIT_AND", offset);
        case OP_BIT_OR:
            return simple_instruction("OP_BIT_OR", offset);
        case OP_BIT_XOR:
            return simple_instruction("OP_BIT_XOR", offset);
        case OP_LEFT_SHIFT:
            return simple_instruction("OP_LEFT_SHIFT", offset);
        case OP_RIGHT_SHIFT:
            return simple_instruction("OP_RIGHT_SHIFT", offset);
        default:
            printf ("Unknown opcode %d\n", instruction);
            return offset + 1;
    }

}