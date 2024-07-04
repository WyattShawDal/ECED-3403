/*
 * File Name: decoding.c
 * Date May 18 2024
 * Written By: Wyatt Shaw
 * Module Info: The module implements the decoding functionality of the emulator, including
 * decoding instructions, printing registers, and modifying registers
 *
 * !!NOTE!! GET_MEM_LOCATION is used only to allow for matching address to the .lis file, it is not needed for functionality
 * and can be removed / adjusted as needed. For demoing purposes it has been left in.
 *
 */

#include "emulation.h"
#include "instruction_table.h"


void print_psw(Emulator *emulator, int style)
{
    if(style == MULTI_LINE)
    {
        printf("CARRY: %d\nOVERFLOW: %d\nNEGATIVE: %d\nZERO: %d\n", emulator->psw.bits.carry, emulator->psw.bits.overflow,
               emulator->psw.bits.negative, emulator->psw.bits.zero);
    }
    else
    {
        printf("PSW --> CARRY: %d OVERFLOW: %d NEGATIVE: %d ZERO: %d\n", emulator->psw.bits.carry, emulator->psw.bits.overflow,
               emulator->psw.bits.negative, emulator->psw.bits.zero);
    }
}

/*
 * @brief This function prints the registers from the reg_file in the emulator
 */
void print_registers(Emulator *emulator)
{

    /* Loop until we reach the base pointer */
    for (int i = 0; i < BASE_PTR; ++i)
    {
        printf("R%d (GPR%d) = %04X \n", i, i, emulator->reg_file[REGISTER][i].word);
    }
    /* prints done separately so we can provide a name for the register */
    printf("R%d (BP)   = %04X \n",BASE_PTR, emulator->reg_file[REGISTER][BASE_PTR].word);
    printf("R%d (LR)   = %04X \n",LINK_REG, emulator->reg_file[REGISTER][LINK_REG].word);
    printf("R%d (SP)   = %04X \n",STACK_PTR, emulator->reg_file[REGISTER][STACK_PTR].word);
    printf("R%d (PC)   = %04X \n",PROG_COUNTER, emulator->reg_file[REGISTER][PROG_COUNTER].word);

}
/*
 * @brief This function allows the user to modify the values of the registers in the emulator
 */

void modify_registers(Emulator *emulator)
{
    unsigned short reg_num = 0;
    unsigned int value= 0;
    printf("Enter Register Number (Decimal): R");
    scanf("%hd", &reg_num);
    if(reg_num < 0 || reg_num > REGFILE_SIZE)
    {
        printf("Register does not exist\n");
    }
    else
    {
        printf("Enter the value to set (Hex): ");
        scanf("%X", &value);
        if(value > 0xFFFF)
        {
            printf("Value to large\n");
        }
        else
        {
            emulator->reg_file[REGISTER][reg_num].word = value;
            printf("Updated Register Value\n");
        }
    }
}
/*
 * @brief This function allows the user to modify the values of the memory in the emulator
 */
void modify_memory_locations(Emulator *emulator)
{
    char mem_type;
    int address;
    unsigned short value;
    printf("Enter a type of memory to modify (I / D): ");
    scanf(" %c", &mem_type);
    mem_type = tolower(mem_type);
    if(mem_type != 'i' && mem_type != 'd')
    {
        printf("Invalid memory type entered\n");
        return;
    }
    //convert to enum
    mem_type = mem_type == 'i' ? I_MEMORY : D_MEMORY;

    printf("Enter address to modify (in hex): ");
    scanf("%x", &address);
    //if memory is I_MEMORY it must be accessed by word
    if(address % 2 != 0 && mem_type == I_MEMORY)
    {
        address -= 1;
        printf("Instruction memory must be accessed by word (even address)\n"
               "Moving address to %04X\n", address);
        return;
    }
    //check if address is valid
    if(address > (WORD_MEMORY_SIZE) || address < 0)
    {
        printf("Invalid address entered");
        return;
    }
    printf("Enter word to set (0-FFFF): ");
    //scan unsigned short as hex
    scanf("%hx", &value);
    //set the value in the memory, since it's a word value we index as a .word
    xm23_memory[mem_type].word[address >> 1] = value;
}
/*
 * @brief This function allows the user to set a breakpoint in the emulator
 */
#define PIPELINE_ADJUSTMENT 4
void set_breakpoint(Emulator *emulator)
{
    //temporay breakpoint to check if it is valid before setting it
    int temp_breakpoint;
    printf("Enter a breakpoint (must be >%04x): ", emulator->reg_file[REGISTER][PROG_COUNTER].word);
    scanf("%x",&temp_breakpoint);
    //if the breakpoint is not even, make it even for word addressing
    temp_breakpoint = (temp_breakpoint % 2 == 0) ? temp_breakpoint : temp_breakpoint - 1;
    //check if program has passed breakpoint already
    //NOTE future implementations could allow this if desired
    if(temp_breakpoint < emulator->reg_file[REGISTER][PROG_COUNTER].word)
    {
        printf("Breakpoint has already been passed\n");
    }
    else
    {
        //add four to adjust breakpoint value given the nops from starting of pipeline
        emulator->breakpoint = temp_breakpoint + PIPELINE_ADJUSTMENT;

        printf("Set breakpoint @ %04x: .LIS value %04x\n", emulator->breakpoint, emulator->breakpoint - PIPELINE_ADJUSTMENT);
    }
}

void parse_cpu_command_block(Emulator *emulator, instruction_data current_instruction, unsigned short starting_address);

/*
 * @brief This function decodes the instructions in the emulator
 * @param emulator the emulator to decode instructions for
 */
void decode_instruction(Emulator *emulator)
{
    if(emulator == NULL)
    {
        printf("Emulator is NULL, exiting program, FATAL ERROR\n");
        exit(-1);
    }
    instruction_data current_instruction;
    //shift starting address right for word addressing since word memory is half the size of byte memory
    current_instruction.word = emulator->instruction_register;

    if(current_instruction.byte[MSB] < ARITHMETIC_UPPER_BOUND && current_instruction.byte[MSB] >= ARITHMETIC_LOWER_BOUND)
    {
        //opcode is only the MSB for this group
        parse_arithmetic_block(emulator, current_instruction, emulator->reg_file[REGISTER][PROG_COUNTER].word);
    }
    else if (current_instruction.byte[MSB] <= REG_MANIP_UPPER_BOUND && current_instruction.byte[MSB] >= REG_MANIP_LOWER_BOUND && ((current_instruction.byte[LSB] & BYTE_MSb) == 0))
    {
        parse_reg_manip_block(emulator,current_instruction, emulator->reg_file[REGISTER][PROG_COUNTER].word);
    }
    else if(current_instruction.byte[MSB] <= REG_MANIP_UPPER_BOUND && current_instruction.byte[MSB] >= REG_MANIP_LOWER_BOUND && ((current_instruction.byte[LSB] & BYTE_MSb) == BYTE_MSb))
    {
        parse_cpu_command_block(emulator, current_instruction, emulator->reg_file[REGISTER][PROG_COUNTER].word);
    }
//load store to be implemented here (past a2)
//        else if (current_instruction.byte[MSB] < 0x60 && current_instruction.byte[MSB] >= 0x58 || current_instruction.byte[MSB] <= 0x9F && current_instruction.byte[MSB] >= 0x80)
//        {
//            parse_load_store(current_instruction, emulator->reg_file[REGISTER][PROG_COUNTER].word);
//        }
    else if(current_instruction.byte[MSB] <= REG_INIT_UPPER_BOUND && current_instruction.byte[MSB] >= REG_INIT_LOWER_BOUND)
    {
        parse_reg_init(emulator, current_instruction,
                       emulator->reg_file[REGISTER][PROG_COUNTER].word);
    }
    else
    {
        if(current_instruction.word != 0x0000) {
//            printf("%04X: NOT SUPPORTED = %04x\n", emulator->reg_file[REGISTER][PROG_COUNTER].word, current_instruction.word);
        }
        else {
//            printf("%04X: NOP.\n", emulator->reg_file[REGISTER][PROG_COUNTER].word);
            emulator->opcode = -1;
        }
    }
}



#define UPPER_BYTE_MASK 0xFF00
#define LOWER_NIBBLE_MASK 0x0F
#define CONSTANT 1
/*
 * @brief This function parses the arithmetic block of instructions
 * @param current_instruction the current instruction to parse
 * @param starting_addr the starting address of the instruction
 * @note This function extracts all the values of the bits used and also assigns
 * an opcode for the emulator to use later in execution
 */
#define GET_MEM_LOCATION(x) (x - 4)
void parse_arithmetic_block(Emulator *emulator, instruction_data current_instruction, short starting_addr)
{
    //emulator->opcode = current_instruction.word & UPPER_BYTE_MASK;
    emulator->opcode = current_instruction.byte[MSB];
    //emulator->opcode = current_instruction.byte[MSB];
    emulator->operand_bits = current_instruction.byte[LSB];
    //extract the bottom nibble of the opcode
    short instruction_table_index = emulator->opcode & LOWER_NIBBLE_MASK;
    /* get opcode from the table */
    emulator->opcode = arithmetic_instruction_table[instruction_table_index].execution_opcode;
//    printf("%4X: %s ", GET_MEM_LOCATION(starting_addr),
//           arithmetic_instruction_table[instruction_table_index].instruction_name);

    //operand bits are the LSB for this block, so the get RC we can just shift all the way to the right
    //[RC]
    emulator->inst_operands.register_or_constant = emulator->operand_bits >> 7,
    emulator->inst_operands.word_or_byte = ((emulator->operand_bits >> 6) & BIT0),
    emulator->inst_operands.source_const  = (emulator->operand_bits >> 3) & EXTRACT_LOW_THREE_BITS,
    emulator->inst_operands.dest = emulator->operand_bits & EXTRACT_LOW_THREE_BITS;

    if(emulator->inst_operands.register_or_constant == CONSTANT)
    {
//        printf("R/C = %d, W/B = %d, CON = %d , DEST = R%d\n", emulator->inst_operands.register_or_constant, emulator->inst_operands.word_or_byte, emulator->inst_operands.source_const, emulator->inst_operands.dest);
    }
    else
    {
//        printf("R/C = %d, W/B = %d, SRC = R%d, DEST = R%d\n", emulator->inst_operands.register_or_constant, emulator->inst_operands.word_or_byte, emulator->inst_operands.source_const, emulator->inst_operands.dest);
    }
}
#define MOV_SWAP 0x0C
#define BYTE_MANIP 0x0D
#define SRA 0x00
#define RRC 0x01
#define SWPB 0x03
#define SXT 0x04

/*
 * @brief This function parses the register manipulation block of instructions
 * @param current_instruction the current instruction to parse
 * @param starting_addr the starting address of the instruction
 *
 */
void parse_reg_manip_block(Emulator *emulator, instruction_data current_instruction, short starting_addr)
{
    emulator->operand_bits = current_instruction.byte[LSB];
    short val = (current_instruction.byte[MSB] & LOWER_NIBBLE_MASK);
    if (val == MOV_SWAP)
    {
        emulator->inst_operands.source_const = (emulator->operand_bits >> 3) & EXTRACT_LOW_THREE_BITS, emulator->inst_operands.dest = emulator->operand_bits & EXTRACT_LOW_THREE_BITS;
        if((current_instruction.byte[LSB] & BIT7) == BIT7)
        {
            emulator->opcode = swap;
//            printf("%04X: SWAP ", GET_MEM_LOCATION(starting_addr));
            unsigned char word_or_byte = 0;
//            printf("W/B = %d, SRC = R%d , DEST = R%d\n", word_or_byte, emulator->inst_operands.source_const, emulator->inst_operands.dest);
        }
        else
        {
            emulator->opcode = mov;
//            printf("%04X: MOV ", GET_MEM_LOCATION(starting_addr));
            unsigned char word_or_byte = (current_instruction.byte[LSB] >> 6) & BIT0;
//            printf("W/B = %d, SRC = R%d , DEST = R%d\n", word_or_byte, emulator->inst_operands.source_const, emulator->inst_operands.dest);
        }
    }
    else if(val == BYTE_MANIP)
    {
        /* Check bits 5-3 to identify function */
        unsigned char comparison_value = (emulator->operand_bits >> 3) & EXTRACT_LOW_THREE_BITS;
        emulator->inst_operands.word_or_byte = (current_instruction.byte[LSB] >> 6) & BIT0;
        emulator->inst_operands.dest = emulator->operand_bits & EXTRACT_LOW_THREE_BITS;
        switch(comparison_value)
        {
            case SRA:
                emulator->opcode = sra;
//                printf("%04X: SRA, DEST = R%d\n",  GET_MEM_LOCATION(starting_addr), emulator->inst_operands.dest);
                break;
            case RRC:
                emulator->opcode = rrc;
//                printf("%04X: RRC, DEST = R%d\n",  GET_MEM_LOCATION(starting_addr), emulator->inst_operands.dest);
                break;
            case SWPB:
                emulator->opcode = swpb;
//                printf("%04X: SWPB, DEST = R%d\n",  GET_MEM_LOCATION(starting_addr), emulator->inst_operands.dest);
                break;
            case SXT:
                emulator->opcode = sxt;
//                printf("%04X: SXT, DEST = R%d\n",  GET_MEM_LOCATION(starting_addr), emulator->inst_operands.dest);
                break;
            default:
                break;
        }
    }
    return;
}
#define SETPRI 0x08
#define SVC 0x09
#define SETCC 0x0A
#define CLRCC 0x0C
#define LOW_5_BITS 0x1F
void parse_cpu_command_block(Emulator *emulator, instruction_data current_instruction, unsigned short starting_address)
{
    //instructions different according to value of bits 7-4
    unsigned short val = (current_instruction.byte[LSB] >> 4 & LOWER_NIBBLE_MASK);
    switch (val)
    {
        case SETPRI:
            //placeholder
            break;
        case SVC:
            //placeholder
            break;
        case SETCC:
        case SETCC+1: //plus one incase overflow bit is set
            emulator->opcode = setcc;
            break;
        case CLRCC:
        case CLRCC+1: //plus one incase overflow bit is set
            emulator->opcode = clrcc;
            break;
    }
    emulator->cpu_ops.byte = current_instruction.byte[LSB] & LOW_5_BITS;
}
/*
 * @brief This function parses the move block of instructions
 * @param current_instruction the current instruction to parse
 * @param starting_addr the starting address of the instruction
 */

void parse_reg_init(Emulator *emulator, instruction_data current_instruction, short starting_addr)
{
    //check bits 12 and 11, shift that value to the right to get a value from 0-4, use that to index into the movement_instruction_table
    short table_index = (current_instruction.word >> 11) & EXTRACT_LOW_TWO_BITS;
    emulator->opcode = movement_instruction_table[table_index].execution_opcode;
    if(emulator->opcode > movh || emulator->opcode < movl)
    {
//        printf("Invalid opcode for reg_init instruction\n");
        return;
    }
    else
    {
//        printf("%04X: %s ",  GET_MEM_LOCATION(starting_addr), movement_instruction_table[table_index].instruction_name);
        //extract bytes to be moved from bits 10-3 and destination from bits 2-0
        emulator->move_byte = (current_instruction.word >> 3) & 0xFF;
        emulator->inst_operands.dest = current_instruction.word & EXTRACT_LOW_THREE_BITS;
//        printf("BYTE = %02x, DEST = R%d\n", emulator->move_byte, emulator->inst_operands.dest);
    }
}
//todo later A3
void parse_load_store(Emulator *emulator, instruction_data current_instruction, unsigned short starting_addr)
{
    emulator->inst_operands.source_const = (current_instruction.word >> 3) & EXTRACT_LOW_THREE_BITS;
    emulator->inst_operands.dest = (current_instruction.word) & EXTRACT_LOW_THREE_BITS;
    emulator->inst_operands.word_or_byte = (current_instruction.word >> 6) & BIT0;
    if(current_instruction.word < 0x60) //indexed addressing
    {

    }
    else //relative addressing
    {

    }
}