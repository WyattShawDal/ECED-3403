/*
 * File Name: main.c
 * Date May 18 2024
 * Written By: Wyatt Shaw
 * Module Info: This module defines global vars, adds support for launching
 * the program with an initial xme file, and then calls menu
 *
 */

#include <stdio.h>
#include <stdbool.h>

#include "decoder.h"
#include "loader.h"


#ifdef TWO_MEM_ARRAY
unsigned char IMEM[BYTE_MEMORY_SIZE];
unsigned char DMEM[BYTE_MEMORY_SIZE];
#else
Memory loader_memory[2];
#endif

Emulator my_emulator;
FILE* input_file;

int main(int argc, char* argv[]) {
    //support for dragging a file onto the executable to start the program
    if (argc > 1)
    {
        printf("File provided to loader (%s), loading now..\n", argv[1]);
        load(input_file, argv[1]);
        menu();
    }
    else
    {
        menu();
    }

    return 0;
}
