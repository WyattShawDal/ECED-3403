//
// Created by wyatt on 2024-05-29.
//

#ifndef ASSIGNMENT1_DECODER_H
#define ASSIGNMENT1_DECODER_H
#include "loader.h"
typedef union instruction_data
{
    unsigned char byte[2];
    unsigned short word;
}instruction_data;
typedef struct instruction
{
    instruction_data data;
    char operands;
}instruction;

void decode_instruction();


extern Memory loader_memory[2];

#endif //ASSIGNMENT1_DECODER_H
