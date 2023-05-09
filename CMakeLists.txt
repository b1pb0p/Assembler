cmake_minimum_required(VERSION 3.24)
project(Assembler C)

set(CMAKE_C_STANDARD 11)

add_executable(Assembler
        assembler.c preprocessor.c preprocessor.h utils.c utils.h errors.c errors.h assembler.h)
