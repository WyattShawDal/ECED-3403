/*
 * File Name: emulation.h
 * Date Created: May 18, 2024
 * Written By: Wyatt Shaw
 * Module Info: The module declares the functions used for the emulator, including functions needed for decoding
 * emulation and execution
 */
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
#define CEX_LOWER_BOUND 0x50
#define CEX_UPPER_BOUND 0x53
#define REG_INIT_LOWER_BOUND 0x60
#define REG_INIT_UPPER_BOUND 0x7F

#define MULTI_LINE 1
#define SINGLE_LINE (-1)

#define EXTRACT_BITS(num_bits, start, val) ((((1 << num_bits) - 1) << start) & val)
#define REG_FILE_OPTIONS 2 //register or constant
#define REGFILE_SIZE 8

#define TEST_BIT(val, bit_pos) (((val) & (bit_pos)) == (bit_pos))

typedef enum
{
    bl = -1,
    beq_bz = 0,
    bne_bnz,
    bc_bhs,
    bnc_blo,
    bn,
    bge,
    blt,
    bra,
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
    cex,
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
/*
 * The cpu_operands_bits struct is used for the setcc clrcc instructions as a means to quickly
 * identify the bits to be changed
 */
typedef struct cpu_operands_bits
{
    unsigned char carry : 1;
    unsigned char zero : 1;
    unsigned char negative : 1;
    unsigned char sleep : 1;
    unsigned char overflow : 1;
}cpu_operands_bits;
/*
 * The cpu_operands union is used to quickly access the bits of the cpu_operands_bits struct
 * it allows the operation |= &=~ to be used to change the bits quickly in execution
 */
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

typedef enum {
    CEX_FALSE = 0,
    CEX_TRUE = 1,
    CEX_DISABLED = 2,
}cex_status;

typedef enum
{
    eq, //equal
    ne, //not equal
    cs, //carry set
    cc, //carry clear
    mi, //minus
    pl, //plus
    vs, //overflow
    vc, //no overflow
    hi, //unsigned higher
    ls, //unsigned lower or same
    ge, //signed greater or equal
    lt, //signed less than
    gt, //signed greater than
    le, //signed less or equal
    tr, //always true
    fl, //always false
}cex_codes;
typedef struct cex_control
{
    cex_codes current_code; //current condition code eq, ne, etc.
    cex_status status; //true, false, disabled
    unsigned short true_count; //if true, number of instructions to execute before skipping false count
    unsigned short false_count; //if false, number of instructions to skip before executing
}CexControl;

typedef struct emulator_data
{
    OPCODES opcode; //opcode of instruction, instructions are 16 bits so this can hold any possible opcode
    short operand_bits; //temp variable for extracting bit values
    cpu_operands cpu_ops; //used when decoding cpu status commands
    operands inst_operands;
    program_status_word psw; //status word bitfield struct
    instruction_data reg_file[REG_FILE_OPTIONS][REGFILE_SIZE]; //contains the xm23p's registers Link, Prog Counter, GBR, etc.
    InstControlRegisters i_control; //registers to emulate xm23p behaviour
    DataControlRegisters d_control;//registers to emulate xm23p behaviour
    HazardControl hazard_control; //contains boolean values for hazard control like bubbling
    CexControl cond_exec; //contains conditional execution control values
    bool is_memset; //bool to check if a file has been loaded
    bool has_started; //bool to check if the emulator has started
    bool is_single_step; //bool to check if the emulator will run in single step mode or continuous
    bool is_user_interrupt; //bool to check if the user has interrupted the emulator via a SIGINT
    bool hide_menu_prompt; //bool to toggle menu prompt visibility, useful for clearly viewing pipeline
    bool stop_on_clock; //bool to toggle if emulator should step via clock tick or a full cycle
    short offset; //offset used in branching instructions
    MEMORY_ACCESS_TYPES xCTRL; //memory access type
    unsigned short instruction_register;
    unsigned char move_byte; //used in mov and swap instructions
    unsigned long int clock; //clock counter
    unsigned int starting_address; //starting address of the emulator
    unsigned int breakpoint; //address of instruction to break at
}Emulator;

void menu(Emulator *emulator);
void init_emulator(Emulator *emulator);
void print_psw(Emulator *emulator, int style);
void print_menu_options();
void execute_branch(Emulator *emulator);
unsigned short do_even_cycle(Emulator *emulator);
void do_odd_cycle(Emulator *emulator, unsigned short previously_decoded);


//decoding
void print_registers(Emulator *emulator);
void modify_registers(Emulator *emulator);
void modify_memory_locations(Emulator *emulator);
void set_breakpoint(Emulator *emulator);
void decode_instruction(Emulator *emulator);
void parse_arithmetic_block(Emulator *emulator, instruction_data current_instruction, short starting_addr);
void parse_reg_manip_block(Emulator *emulator, instruction_data current_instruction, short starting_addr);
void parse_reg_init(Emulator *emulator, instruction_data current_instruction);
void parse_load_store(Emulator *emulator, instruction_data current_instruction);
void parse_cpu_command_block(Emulator *emulator, instruction_data current_instruction);
void parse_branch_block(Emulator *emulator, instruction_data data);
void parse_cex_instruction(Emulator *emulator, instruction_data data);
//executions
void update_psw(unsigned short result, Emulator *emulator, unsigned short old_dest, unsigned short source);
void execute_1(Emulator *emulator);
void execute_0(Emulator *emulator);
void fetch_instruction(Emulator *emulator, int even);
void memory_controller(Emulator *emulator);
void do_cex(Emulator *emulator, cex_status status);
void run_emulator(Emulator *emulator);
void bcd_addition(Emulator *emulator);
short calc_index_adjustment(Emulator *emulator);

void execute_arithmetic(Emulator *emulator);
void execute_chg_reg(Emulator *emulator);
void execute_load_store(Emulator *emulator);
void execute_reg_manip(Emulator *emulator);
void execute_mov_swap(Emulator *emulator);
void execute_cex(Emulator *emulator);



extern Memory loader_memory[2];

#endif //ASSIGNMENT1_DECODER_H
