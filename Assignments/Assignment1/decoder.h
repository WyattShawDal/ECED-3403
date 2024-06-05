//
// Created by wyatt on 2024-05-29.
//

#ifndef ASSIGNMENT1_DECODER_H
#define ASSIGNMENT1_DECODER_H
#include "loader.h"

typedef enum
{
    B7 = 0x80,
    B6 = 0x40,
    B5 = 0x20,
    B4 = 0x10,
    B3 = 0x08,
    B2 = 0x04,
    B1 = 0x02,
    B0 = 0x01,
    EXTRACT_LOW_THREE_BITS = 0x07,
    EXTRACT_LOW_TWO_BITS = 0x03,
}BITVALS;
typedef union instruction_data
{
    unsigned char byte[2];
    unsigned short word;
}instruction_data;
typedef struct emulator_data
{
    short opcode;
    short operands;
    short program_counter;
}Emulator;


void decode_instruction();
void parse_arithmetic_block(instruction_data current_instruction, short starting_addr);
void parse_reg_manip_block(instruction_data current_instruction, short starting_addr);
void parse_move_block(instruction_data current_instruction, short starting_addr);


extern Memory loader_memory[2];

#endif //ASSIGNMENT1_DECODER_H
