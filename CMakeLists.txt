# fake cmakelist

cmake_minimum_required(VERSION 3.10)

project(MIAO)

file(GLOB_RECURSE ALL_C_FILES *.c */*.c)
file(GLOB_RECURSE ALL_S_FILES *.S */*.S)
file(GLOB_RECURSE ALL_HEADER_FILES *.h */*.h)
add_executable(ALL_FILES ${ALL_C_FILES} ${ALL_S_FILES} ${ALL_HEADER_FILES} student-distrib/GUI/modex.c student-distrib/GUI/modex.h)
add_custom_target(NULL)