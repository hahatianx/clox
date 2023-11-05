cmake_minimum_required(VERSION 3.15.0)

project(clox)

set(EXECUTABLE_OUTPUT_PATH ../bin)


file(GLOB_RECURSE C_SRC ${PROJECT_SOURCE_DIR}/src/*.c)
list(FILTER C_SRC EXCLUDE REGEX ".*main.c")

file(GLOB_RECURSE MAIN ${PROJECT_SOURCE_DIR}/src/main.c)
file(GLOB_RECURSE TEST_MAIN ${PROJECT_SOURCE_DIR}/src/test_main.c)

include_directories(${PROJECT_SOURCE_DIR}/include)


add_executable(clox ${MAIN} ${C_SRC})
add_executable(test_clox ${TEST_MAIN} ${C_SRC})