#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include "common.h"

#include "basic/chunk.h"
#include "debug/debug.h"

#include "vm/vm.h"
#include "vm/scanner.h"

static void repl() {
    char line[1024];
    for (;;) {
        printf(">>> ");
        if (!fgets(line, sizeof line, stdin)) {
            printf("\n");
            break;
        }
        if (!strncmp(line, "exit", 4)) {
            break;
        }
        interpret(line);
    }
}

static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytes_left = file_size;
    size_t file_pointer = 0;
    while (bytes_left) {
        size_t bytes_read = fread(buffer + file_pointer, sizeof(char), bytes_left, file);
        if (bytes_read < bytes_left) {
            if (errno != EINTR) {
                fprintf(stderr, "Cloud not read file \"%s\".\n", path);
                exit(74);
            }
        }
        file_pointer += bytes_read;
        bytes_left -= bytes_read;
    }
    buffer[file_size] = '\0';

    fclose(file);
    return buffer;
}

static void run_file(const char* path) {
    char* source = read_file(path);
    interpret_result_t result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

void launch_interpreter() {
    launch_scanner();
    init_vm();

}

void shutdown_interpreter() {
    free_vm();
    free_scanner();
}

int main(int argc, const char **argv) {

    launch_interpreter();

    if (argc == 1) {
        repl();
    } else {
        run_file(argv[1]);
    }

    shutdown_interpreter();
    return 0;
}
