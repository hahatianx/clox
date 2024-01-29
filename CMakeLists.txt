cmake_minimum_required(VERSION 3.15.0)

project(clox)

add_compile_options("-Wall")
add_definitions(-std=c17)

set(EXECUTABLE_OUTPUT_PATH ../bin)


file(GLOB_RECURSE C_SRC ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE C_INC ${PROJECT_SOURCE_DIR}/include/*)
list(FILTER C_SRC EXCLUDE REGEX ".*main.c")

file(GLOB_RECURSE MAIN ${PROJECT_SOURCE_DIR}/src/main.c)
file(GLOB_RECURSE TEST_MAIN ${PROJECT_SOURCE_DIR}/src/test_main.c)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(clox ${MAIN} ${C_SRC} ${C_INC}
        include/value/native/clock.h
        src/value/native/clock.c
        include/vm/runtime.h
        include/utils/stack.h
        src/utils/stack.c
        src/component/graystack.c
        include/component/graystack.h
        include/switch.h
        src/switch.c
        include/value/object/class.h
        src/value/object/class.c
        include/value/object/list.h
        src/value/object/list.c)
add_executable(test_clox ${TEST_MAIN} ${C_SRC}
        src/value/native/clock.c
        include/vm/runtime.h)