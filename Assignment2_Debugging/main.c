/*
 * File Name: main.c
 * Date June 20 2024
 * Written By: Wyatt Shaw
 * Module Info: This module defines global vars, adds support for launching
 * the program with an initial xme file, and then calls menu
 *
 */

#include <stdio.h>
#include <stdbool.h>

#include "emulation.h"
#include "loader.h"

Memory xm23_memory[2];
FILE* input_file;

int main(int argc, char* argv[]) {
    Emulator *new_emulator = calloc(1, sizeof(Emulator));
    //support for dragging a file onto the executable to start the program
    init_emulator(new_emulator);
    if (argc > 1)
    {
        printf("File provided to loader (%s), loading now..\n", argv[1]);
        load(input_file, argv[1], new_emulator);
        menu(new_emulator);
    }
    else
    {
        menu(new_emulator);
    }
    return 0;
}
