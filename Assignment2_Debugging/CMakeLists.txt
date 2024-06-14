cmake_minimum_required(VERSION 3.28)
project(Assignment2_Debugging C)

set(CMAKE_C_STANDARD 11)

add_executable(Assignment2_Debugging main.c
        loader.c
        decoding.c
        execution.c
        loader.h
        emulation.h
        instruction_table.h
        emulation.c
)
