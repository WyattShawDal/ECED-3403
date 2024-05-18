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

unsigned char IMEM[xMEM_SIZE];
unsigned char DMEM[xMEM_SIZE];

FILE* input_file;

int main(int argc, char* argv[]) {
    if (argc > 1) //support for dragging a file onto the executable to start
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
