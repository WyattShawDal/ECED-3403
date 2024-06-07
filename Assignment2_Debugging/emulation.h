//
// Created by wyatt on 2024-05-29.
//

#ifndef ASSIGNMENT1_DECODER_H
#define ASSIGNMENT1_DECODER_H
#include "loader.h"

#define REGISTER 0
#define CONSTANT 1
#define MSB 1
#define LSB 0


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

typedef enum REGISTERS
{
    GRP0 = 0,
    GPR1 = 1,
    GPR2 = 2,
    GPR3 = 3,
    BASE_PTR = 4,
    LINK_REG = 5,
    STACK_PTR = 6,
    PROG_COUNTER = 7,
}REGISTERS;

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

typedef struct nibbles
{
    unsigned short nib1 : 4;
    unsigned short nib2 : 4;
    unsigned short nib3 : 4;
    unsigned short nib4 : 4;
}nibbles;

typedef union word_nibbles {
    unsigned short word;
    struct nibbles nibbles;
}word_nibbles;

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


typedef struct program_status_word
{
    unsigned short previous_prio :3;
    unsigned short unused :4;
    unsigned short fault :1;
    unsigned short current_prio :3;
    unsigned short overflow :1;
    unsigned short sleep :1;
    unsigned short negative :1;
    unsigned short zero :1;
    unsigned short carry :1;
}program_status_word;

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
    instruction_data reg_file[REG_CON][REGFILE];
    unsigned int breakpoint;
    program_status_word psw;
}Emulator;
void debugger_menu();


//decoding
void print_registers();
void modify_registers();
void modify_memory_locations();
void set_breakpoint();
void decode_instruction();
void parse_arithmetic_block(instruction_data current_instruction, short starting_addr);
void parse_reg_manip_block(instruction_data current_instruction, short starting_addr);
void parse_move_block(instruction_data current_instruction, short starting_addr);


//executions
void update_psw(unsigned short result, Emulator *emulator);
void execute(Emulator *emulator);


extern Memory loader_memory[2];

#endif //ASSIGNMENT1_DECODER_H
