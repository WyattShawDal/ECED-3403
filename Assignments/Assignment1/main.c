#include <stdio.h>

#include "loader.h"

unsigned char IMEM[xMEM_SIZE];
unsigned char DMEM[xMEM_SIZE];

FILE* input_file;

int main(int argc, char* argv[]) {
    if (argc > 1)
    {
        printf("File provided to loader (%s), loading now..", argv[2]);
        load(input_file, argv[2]);
    }
    else
    {
        menu();
    }

    return 0;
}
