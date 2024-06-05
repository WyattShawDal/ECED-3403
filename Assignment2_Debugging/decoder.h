//
// Created by wyatt on 2024-05-29.
//

#ifndef ASSIGNMENT1_DECODER_H
#define ASSIGNMENT1_DECODER_H
#include "loader.h"

typedef enum
{
    add,
    addc,
    sub,
    subc,
    dadd,
    cmp,
    xor,
    and,
    or,
    bit,
    bic,
    bis,
    mov,
    swap,
    sra,
    rrc,
    swpb,
    sxt,
    movl,
    movlz,
    movls,
    movh

}OPCODES;

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
typedef struct
{
    unsigned char register_or_constant;
    unsigned char word_or_byte;
    unsigned char source_const;
    unsigned char dest;
}arithmetic;
typedef struct
{
    unsigned char register_or_constant;
    unsigned char word_or_byte;
    unsigned char source_const;
    unsigned char dest;
}reg_manip;
typedef struct
{
    unsigned char byte;
    unsigned char dest : 3; //not sure best way to use this
}move;
typedef union operands
{
    arithmetic arithmetic_operands;
    reg_manip reg_manip_operands;
    move move_operands;
}operands;
#define REG_CON 2

#define REGFILE 8
typedef struct emulator_data
{
    short opcode;
    short operands;
    union operands my_operands;
    unsigned int program_counter;
    bool is_memset;
    bool is_emulator_running;
    unsigned int clock;
    unsigned int starting_address;
    bool is_single_step;
    unsigned short reg_file[REG_CON][REGFILE];
    unsigned int breakpoint;
}Emulator;

void debugger_menu();
void print_registers();
void modify_registers();
void modify_memory_locations();
void set_breakpoint();
void decode_instruction();
void parse_arithmetic_block(instruction_data current_instruction, short starting_addr);
void parse_reg_manip_block(instruction_data current_instruction, short starting_addr);
void parse_move_block(instruction_data current_instruction, short starting_addr);


extern Memory loader_memory[2];

#endif //ASSIGNMENT1_DECODER_H
