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
#define BYTE_SHIFT 7
#define WORD_SHIFT 15

#define ARITHMETIC_LOWER_BOUND 0x40
#define ARITHMETIC_UPPER_BOUND 0x4C
#define REG_MANIP_LOWER_BOUND 0x4C
#define REG_MANIP_UPPER_BOUND 0x4D
#define REG_INIT_LOWER_BOUND 0x60
#define REG_INIT_UPPER_BOUND 0x7F

#define MULTI_LINE 1
#define SINGLE_LINE (-1)

#define TEST_BIT(val, bit_pos) (((val) & (bit_pos)) == (bit_pos))

typedef enum
{
    //todo if things break
    bl = -1,
    beq_bz = 0,
    bne_bnz,
    bc_bhs,
    bnc_blo,
    bn,
    bge,
    blt,
    bra,
    //todo this should be set to 1
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
    setcc,
    clrcc,
    ld,
    st,
    ldr,
    str,
    movl,
    movlz,
    movls,
    movh,

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
    WORD_MSb = 0x8000,
    BYTE_MSb = 0x80,
    BIT15 = 0x8000,
    BIT14 = 0x4000,
    BIT13 = 0x2000,
    BIT12 = 0x1000,
    BIT11 = 0x800,
    BIT10 = 0x400,
    BIT9 = 0x200,
    BIT8 = 0x100,
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
    unsigned short nib0 : 4;
    unsigned short nib1 : 4;
    unsigned short nib2 : 4;
    unsigned short nib3 : 4;
}nibbles;
//
//[ 4][ 3][ 2][ 1]
//[ SHORT MEMORY ]
typedef union word_nibbles {
    unsigned short word;
    struct nibbles nibbles;
}word_nibbles;

typedef union instruction_data
{
    unsigned char byte[2];
    unsigned short word;
}instruction_data;


typedef struct operands{
    unsigned short dest : 3;
    unsigned short source_const : 3;
    unsigned short register_or_constant : 1;
    unsigned short word_or_byte : 1;
    unsigned short inc : 2;
    unsigned short dec : 2;
    unsigned short prpo : 1;
}operands;
typedef struct cpu_operands_bits
{
    unsigned char carry : 1;
    unsigned char zero : 1;
    unsigned char negative : 1;
    unsigned char sleep : 1;
    unsigned char overflow : 1;
}cpu_operands_bits;
typedef union cpu_operands {
    unsigned char byte;
    struct cpu_operands_bits bits;
}cpu_operands;

typedef struct program_status_word_bits
{
    unsigned short carry :1;
    unsigned short zero :1;
    unsigned short negative :1;
    unsigned short sleep :1;
    unsigned short overflow :1;
    unsigned short current_prio :3;
    unsigned short fault :1;
    unsigned short unused :4;
    //later use
    unsigned short previous_prio :3;
}program_status_word_bits;

typedef union program_status_word {
    unsigned short word;
    struct program_status_word_bits bits;
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
typedef struct hazard_control {
    bool d_bubble;
    bool e_bubble;
}HazardControl;

#define EXTRACT_BITS(num_bits, start, val) ((((1 << num_bits) - 1) << start) & val)


#define REG_FILE_OPTIONS 2 //register or constant
#define REGFILE_SIZE 8
typedef struct emulator_data
{
    OPCODES opcode; //opcode of instruction, instructions are 16 bits so this can hold any possible opcode
    short operand_bits; //temp variable for extracting bit values
    cpu_operands cpu_ops;
    operands inst_operands;
    program_status_word psw; //status word bitfield struct
    instruction_data reg_file[REG_FILE_OPTIONS][REGFILE_SIZE];
    InstControlRegisters i_control; //registers to emulate xm23p behaviour
    DataControlRegisters d_control;//registers to emulate xm23p behaviour
    HazardControl hazard_control;
    bool is_memset; //bool to check if a file has been loaded
    bool has_started; //bool to check if the emulator has started
    bool is_single_step; //bool to check if the emulator will run in single step mode or continuous
    bool is_user_interrupt; //bool to check if the user has interrupted the emulator via a SIGINT
    bool hide_menu_prompt;
    bool stop_on_clock;
    short offset;
    MEMORY_ACCESS_TYPES xCTRL;
    unsigned short instruction_register;
    unsigned char move_byte;
    unsigned long int clock;
    unsigned int starting_address;
    unsigned int breakpoint;
}Emulator;
void menu(Emulator *emulator);
void init_emulator(Emulator *emulator);
void print_psw(Emulator *emulator, int style);
void print_menu_options();
void execute_branch(Emulator *emulator);

//decoding
void print_registers(Emulator *emulator);
void modify_registers(Emulator *emulator);
void modify_memory_locations(Emulator *emulator);
void set_breakpoint(Emulator *emulator);
void decode_instruction(Emulator *emulator);
void parse_arithmetic_block(Emulator *emulator, instruction_data current_instruction, short starting_addr);
void parse_reg_manip_block(Emulator *emulator, instruction_data current_instruction, short starting_addr);
void parse_reg_init(Emulator *emulator, instruction_data current_instruction);
//later use (not a2)
void parse_load_store(Emulator *emulator, instruction_data current_instruction);

void parse_cpu_command_block(Emulator *emulator, instruction_data current_instruction);

void parse_branch_block(Emulator *emulator, instruction_data data);


//executions
void
update_psw(unsigned short result, Emulator *emulator, unsigned short old_dest, unsigned short source);
void execute_1(Emulator *emulator);
void execute_0(Emulator *emulator);
void fetch_instruction(Emulator *emulator, int even);
void memory_controller(Emulator *emulator);
void run_emulator(Emulator *emulator);
void bcd_addition(Emulator *emulator);
short calc_index_adjustment(Emulator *emulator);

void execute_arithmetic(Emulator *emulator);
void execute_chg_reg(Emulator *emulator);
void execute_load_store(Emulator *emulator);
void execute_reg_manip(Emulator *emulator);
void execute_mov_swap(Emulator *emulator);



extern Memory loader_memory[2];

#endif //ASSIGNMENT1_DECODER_H
