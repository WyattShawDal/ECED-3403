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

#include "loader.h"
#include "decoder.h"


#ifdef TWO_MEM_ARRAY
unsigned char IMEM[BYTE_MEMORY_SIZE];
unsigned char DMEM[BYTE_MEMORY_SIZE];
#else
Memory loader_memory[2];
#endif

#define REG_CON 2
#define REGFILE 8

Emulator my_emulator;
FILE* input_file;

void init_emulator();

int main(int argc, char* argv[]) {
    //support for dragging a file onto the executable to start the program
    init_emulator();
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

void init_emulator()
{
    //initalize breakpoint to the end of the memory space
    my_emulator.breakpoint = (BYTE_MEMORY_SIZE) - 1;
    unsigned short reg_file[2][8] = {{0, 0, 0, 0, 0, 0, 0, 0},
                             {0, 1, 2, 4, 8, 16, 32, -1}};
    memcpy(my_emulator.reg_file, reg_file, sizeof (unsigned short) * REG_CON * REGFILE);
}