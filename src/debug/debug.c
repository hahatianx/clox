#include <stdio.h>

#include "debug/debug.h"
#include "value/value.h"

static int simple_instruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int two_byte_instruction(const char* name, chunk_t* chunk, int offset) {
    printf("%-20s ", name);
    printf("%4d\n", chunk->code[offset + 1]);
    return offset + 2;
}

static int constant_instruction(const char* name, chunk_t* chunk, int offset) {
    uint8_t constant = chunk->code[offset + 1];
    printf("%-20s %4d '", name, constant);
    print_value(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

static int byte_instruction(const char* name, chunk_t* chunk, int offset) {
    uint8_t slot = chunk->code[offset + 1];
    printf("%-20s %4d\n", name, slot);
    return offset + 2;
}

static int constant_instruction_long(const char* name, chunk_t* chunk, int offset) {
    uint8_t constant_1 = chunk->code[offset + 1];
    uint8_t constant_2 = chunk->code[offset + 2];
    int constant = (constant_1 << 8) | (constant_2);
    printf("%-20s %4d '", name, constant);
    print_value(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 3;
}

static int jump_instruction(const char* name, int sign, chunk_t* chunk, int offset) {
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset + 2];
    printf("%-20s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
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
        case OP_SUBTRACT:
            return simple_instruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:
            return simple_instruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simple_instruction("OP_DIVIDE", offset);
        case OP_NOT:
            return simple_instruction("OP_NOT", offset);
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
        case OP_PRINTLN:
            return simple_instruction("OP_PRINTLN", offset);
        case OP_POP:
            return simple_instruction("OP_POP", offset);
        case OP_POPN:
            return two_byte_instruction("OP_POPN", chunk, offset);
        case OP_DEFINE_GLOBAL:
            return constant_instruction("OP_DEFINE_GLOBAL", chunk, offset);
        case OP_DEFINE_MUT_GLOBAL:
            return constant_instruction("OP_DEFINE_MUT_GLOBAL", chunk, offset);
        case OP_GET_GLOBAL:
            return constant_instruction("OP_GET_GLOBAL", chunk, offset);
        case OP_SET_GLOBAL:
            return constant_instruction("OP_SET_GLOBAL", chunk, offset);
        case OP_DEFINE_GLOBAL_LONG:
            return constant_instruction_long("OP_DEFINE_GLOBAL_LONG", chunk, offset);
        case OP_DEFINE_MUT_GLOBAL_LONG:
            return constant_instruction("OP_DEFINE_MUT_GLOBAL_LONG", chunk, offset);
        case OP_GET_GLOBAL_LONG:
            return constant_instruction_long("OP_GET_GLOBAL_LONG", chunk, offset);
        case OP_SET_GLOBAL_LONG:
            return constant_instruction_long("OP_SET_GLOBAL_LONG", chunk, offset);
        case OP_DEFINE_LOCAL:
            return simple_instruction("OP_DEFINE_LOCAL", offset);
        case OP_DEFINE_MUT_LOCAL:
            return simple_instruction("OP_DEFINE_MUT_LOCAL", offset);
        case OP_GET_LOCAL:
            return byte_instruction("OP_GET_LOCAL", chunk, offset);
        case OP_SET_LOCAL:
            return byte_instruction("OP_SET_LOCAL", chunk, offset);
        case OP_CLASS:
            return constant_instruction("OP_CLASS", chunk, offset);
        case OP_CLASS_LONG:
            return constant_instruction_long("OP_CLASS_LONG", chunk, offset);
        case OP_METHOD:
            return constant_instruction("OP_METHOD", chunk, offset);
        case OP_METHOD_LONG:
            return constant_instruction_long("OP_METHOD_LONG", chunk, offset);
        case OP_ARRAY:
            return simple_instruction("OP_ARRAY", offset);
        case OP_GET_PROPERTY:
            return constant_instruction("OP_GET_PROPERTY", chunk, offset);
        case OP_GET_PROPERTY_LONG:
            return constant_instruction_long("OP_GET_PROPERTY_LONG", chunk, offset);
        case OP_SET_PROPERTY:
            return constant_instruction("OP_SET_PROPERTY", chunk, offset);
        case OP_SET_PROPERTY_LONG:
            return constant_instruction_long("OP_SET_PROPERTY_LONG", chunk, offset);
        case OP_GET_ARRAY_INDEX:
            return simple_instruction("OP_GET_ARRAY_INDEX", offset);
        case OP_SET_ARRAY_INDEX:
            return simple_instruction("OP_SET_ARRAY_INDEX", offset);
        case OP_GET_UPVALUE:
            return byte_instruction("OP_GET_UPVALUE", chunk, offset);
        case OP_SET_UPVALUE:
            return byte_instruction("OP_SET_UPVALUE", chunk, offset);
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
        case OP_JUMP:
            return jump_instruction("OP_JUMP", 1, chunk, offset);
        case OP_JUMP_IF_FALSE:
            return jump_instruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
        case OP_LOOP:
            return jump_instruction("OP_LOOP", -1, chunk, offset);
        case OP_CALL:
            return byte_instruction("OP_CALL", chunk, offset);
        case OP_CLOSURE_UPVALUE:
            return simple_instruction("OP_CLOSURE_UPVALUE", offset);
        case OP_CLOSURE: {
            offset++;
//            uint16_t constant = (chunk->code[offset] << 8) | chunk->code[offset + 1];
            uint8_t constant = chunk->code[offset++];
            printf("%-20s %4d ", "OP_CLOSURE", constant);
            print_value(chunk->constants.values[constant]);
            printf("\n");
//            offset += 2;

            object_function_t *func = AS_FUNCTION(chunk->constants.values[constant]);
            for (int j = 0; j < func->upvalue_count; ++j) {
                int is_local = chunk->code[offset++];
                int index = chunk->code[offset++];
                printf("%04d      |           %-10s  %d\n",
                       offset-2, is_local ? "local" : "upvalue", index);
            }

            return offset;
        }
        default:
            printf ("Unknown opcode %d\n", instruction);
            return offset + 1;
    }

}

void disassemble_locals(compiler_t* compiler, const char* name) {
    printf("== %s ==\n", name);
    for (int i = 0; i < compiler->local_count; ++i) {
        disassemble_local_var(i, &compiler->locals[i]);
    }
}

void disassemble_local_var(int slot, local_t* local) {
    printf("slot: %d, depth: %d, name: %.*s\n",
        slot, local->depth, local->name.length, local->name.start);
}