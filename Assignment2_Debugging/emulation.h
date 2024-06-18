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
    add =1 ,
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
    BIT7 = 0x80,
    BIT6 = 0x40,
    BIT5 = 0x20,
    BIT4 = 0x10,
    BIT3 = 0x08,
    BIT2 = 0x04,
    BIT1 = 0x02,
    BIT0 = 0x01,
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
//#define FLAG_V1
#ifdef FLAG_V1
typedef union operands
{
    arithmetic arithmetic_operands;
    reg_manip reg_manip_operands;
    move move_operands;
}operands;
#else
typedef struct operands{
    unsigned short inc : 1;
    unsigned short dec : 1;
    unsigned short prpo : 1;
    unsigned short register_or_constant : 1;
    unsigned short word_or_byte : 1;
    unsigned short source_const : 3;
    unsigned short dest : 3;
}operands;

#endif

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

typedef struct i_control_registers
{
    unsigned short IMAR; //program counter value
    unsigned short IMBR; //instruction from fetch
}InstControlRegisters;
typedef struct d_control_registers
{
    unsigned short DMAR; //program counter value
    unsigned short DMBR; //instruction from fetch
}DataControlRegisters;


#define REG_CON 2
#define REGFILE_SIZE 8
typedef struct emulator_data
{
    short opcode;
    short operand_bits; //change
    operands my_operands; //rename
    program_status_word psw;
    instruction_data reg_file[REG_CON][REGFILE_SIZE];
    InstControlRegisters i_control;
    DataControlRegisters d_control;
    unsigned char xCTRL;
    unsigned short instruction_register;
    bool is_memset;
    bool is_emulator_running;
    bool is_single_step;
    unsigned char move_byte;
    unsigned int clock;
    unsigned int starting_address;
    unsigned int breakpoint;
}Emulator;
void debugger_menu();


//decoding
void print_registers();
void modify_registers();
void modify_memory_locations();
void set_breakpoint();
void decode_instruction(Emulator *emulator);
void parse_arithmetic_block(instruction_data current_instruction, short starting_addr);
void parse_reg_manip_block(instruction_data current_instruction, short starting_addr);
void parse_reg_init(instruction_data current_instruction, short starting_addr);
void parse_load_store(instruction_data current_instruction, unsigned short starting_addr);



//executions
void
update_psw(unsigned short result, Emulator *emulator, unsigned short old_dest);
void execute_instruction(Emulator *emulator);
void fetch_instruction(Emulator *emulator, int even);
void memory_controller(Emulator *emulator);
void run_emulator(Emulator *emulator);




extern Memory loader_memory[2];

#endif //ASSIGNMENT1_DECODER_H
