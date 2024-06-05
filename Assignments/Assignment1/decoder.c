/*
 * File Name: decoder.c
 * Date May 30 2024
 * Written By: Wyatt Shaw
 * Module Info: This module implements an initial version of the XM23p instruction decoder.
 * It does not include support for all instructions and does not support further steps in the pipeline.
 * These additions will be added for assignment 2
 * - NOTE: A simple "Emulator" struct is implemented primarily for ideation of future implementations, it does not
 * represent a final version. It will be significantly refactored in the future
 */


#include "decoder.h"
#include "instruction_table.h"

extern Emulator my_emulator;

#define MSB 1
#define LSB 0
void decode_instruction()
{
    instruction_data current_instruction;
    //set starting address to the starting address of the instruction memory,
    //set by s9 record in loader
    short starting_addr = my_emulator.program_counter;
    current_instruction.word = 0x0001;
    //not at end of instruction memory
    while(current_instruction.word != 0x0000)
    {
        //shift starting address right for word addressing since word memory is half the size of byte memory
        current_instruction.word = loader_memory[I_MEMORY].word[starting_addr >> 1];
        if(current_instruction.byte[MSB] < 0x4C && current_instruction.byte[MSB] >= 0x40)
        {
            parse_arithmetic_block(current_instruction, starting_addr);
        }
        else if (current_instruction.byte[MSB] <= 0x4D && current_instruction.byte[MSB] >= 0x4C)
        {
            parse_reg_manip_block(current_instruction, starting_addr);
        }
        else if(current_instruction.byte[MSB] >= 0x60 && current_instruction.byte[MSB] < 0x80)
        {
            parse_move_block(current_instruction, starting_addr);
        }
        else
        {
            if(current_instruction.word == 0x0000)
            {
                printf("%04x: No more instructions in block.\n", starting_addr);
            }
            else
            {
                printf("%04X: NOT SUPPORTED = %04x\n", starting_addr, current_instruction.word);
            }
        }
        starting_addr+= 2;
    }
}

#define UPPER_BYTE_MASK 0xFF00
#define LOWER_NIBBLE_MASK 0x0F
#define CONSTANT 1

void parse_arithmetic_block(instruction_data current_instruction, short starting_addr)
{
    //my_emulator.opcode = current_instruction.word & UPPER_BYTE_MASK;
    my_emulator.opcode = current_instruction.byte[MSB];
    my_emulator.operands = current_instruction.byte[LSB];
    //extract the bottom nibble of the opcode
    short instruction_table_index = my_emulator.opcode & LOWER_NIBBLE_MASK;
    if(arithmetic_instruction_table[instruction_table_index].opcode)
    {
        unsigned char register_or_constant = my_emulator.operands >> 7, word_or_byte = ((my_emulator.operands >> 6) & B0),
        source_const = (my_emulator.operands >> 3) & EXTRACT_LOW_THREE_BITS, dest = my_emulator.operands & EXTRACT_LOW_THREE_BITS;
        char* var_type = register_or_constant== CONSTANT ? "CON" : "SRC";
        char* rc = (register_or_constant== CONSTANT) ? "" : "R";
        printf("%04X: %s R/C = %d, W/B = %d, %s = %s%d , DEST = R%d\n", starting_addr,
               arithmetic_instruction_table[instruction_table_index].instruction_name,register_or_constant,
               word_or_byte,var_type,rc, source_const, dest);
    }
}
#define MOV_SWAP 0x0C
#define BYTE_MANIP 0x0D
#define SRA 0x00
#define RRC 0x01
#define SWPB 0x03
#define SXT 0x04
void parse_reg_manip_block(instruction_data current_instruction, short starting_addr)
{
    my_emulator.opcode = current_instruction.word & UPPER_BYTE_MASK;
    my_emulator.operands = current_instruction.byte[LSB];
    short val = (current_instruction.byte[MSB] & LOWER_NIBBLE_MASK);
    if (val == MOV_SWAP)
    {
        unsigned char src_const = (my_emulator.operands >> 3) & EXTRACT_LOW_THREE_BITS, dest = my_emulator.operands & EXTRACT_LOW_THREE_BITS;
        printf("%04X: %s W/B = %d, SRC = R%d , DEST = R%d\n",starting_addr,
               ((current_instruction.byte[LSB] & B7) == B7) ? "SWAP" : "MOV",
               ((current_instruction.byte[LSB] & B7) == B7) ? 0 : (current_instruction.byte[LSB] >> 6) & B0, src_const, dest);
    }
    else if(val == BYTE_MANIP)
    {
        unsigned char comparison_value = (my_emulator.operands >> 3) & EXTRACT_LOW_THREE_BITS;
        unsigned char dest = my_emulator.operands & EXTRACT_LOW_THREE_BITS;
        char* instr_type;
        switch(comparison_value)
        {
            case SRA:
                instr_type = "SRA";
                break;
            case RRC:
                instr_type = "RRC";
                break;
            case SWPB:
                instr_type = "SWPB";
                break;
            case SXT:
                instr_type = "SXT";
                break;
            default:
                instr_type = "UNK";
                break;
        }
        printf("%04X: %s, DEST = R%d\n", starting_addr, instr_type, dest);
    }
}

void parse_move_block(instruction_data current_instruction, short starting_addr)
{
    //check bits 12 and 11, shift that value to the right to get a value from 0-4, use that to index into the movement_instruction_table
    short table_index = (current_instruction.word >> 11) & EXTRACT_LOW_TWO_BITS;
    if(movement_instruction_table[table_index].opcode > 4 || movement_instruction_table[table_index].opcode < 0)
    {
        printf("Invalid opcode\n");
        return;
    }
    else
    {
        //extract bytes to be moved from bits 10-3 and destination from bits 2-0
        unsigned char bits_to_move = (current_instruction.word >> 3) & 0xFF;
        unsigned char dest = current_instruction.word & EXTRACT_LOW_THREE_BITS;
        printf("%04X: %s BYTE = %02x, DEST = R%d\n", starting_addr, movement_instruction_table[table_index].instruction_name,
               bits_to_move, dest);
    }

}