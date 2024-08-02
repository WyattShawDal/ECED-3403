cmake_minimum_required(VERSION 3.26)
project(XM23p_ECED3403 C)

set(CMAKE_C_STANDARD 11)

add_executable(XM23p main.c
        loader.c
        decoding.c
        execution.c
        loader.h
        emulation.h
        instruction_table.h
        emulation.c
)
