//
// Created by wyatt on 2024-05-29.
//
#include "emulation.h"
#include "instruction_table.h"
extern Emulator my_emulator;


/*
 * @brief This function provides a menu for various debugging options in the
 * emulator
 */
void debugger_menu()
{
    //debugger menu notification
    printf("                ====== Entered Debug Menu ======\n");
    char command;
    do {
        printf("Print Decode (D), Print Registers (R), Modify Register Values (T), Modify Memory Value (U),Set Breakpoint Value (Y), Quit (Q)\n");
        if (scanf(" %c", &command) != 1)
        {
            //clearing the input buffer
            while (getchar() != '\n');
            continue;
        }
        command = (char) tolower(command);
        switch (command)
        {
            case 'd':
                decode_instruction(NULL);
                break;
            case 'r':
                print_registers();
                break;
            case 't':
                modify_registers();
                break;
            case 'u':
                modify_memory_locations();
                break;
            case 'y':
                set_breakpoint();
                break;
            case 'q':
                printf("quitting debug..\n");
                break;
            default:
                printf("Unexpected Command\n");
                break;
        }
        while (getchar() != '\n'); //another buffer clear to ensure menu doesn't loop
    }while (tolower(command) != 'q');
}

/*
 * @brief This function prints the registers from the reg_file in the emulator
 */
void print_registers()
{
    int i = 0;
    /* Loop until we reach the base pointer */
    for (; i < BASE_PTR; ++i)
    {
        printf("R%d = %04x (GPR%d)\n", i, my_emulator.reg_file[REGISTER][i].word, i);
    }
    /* prints done seperately so we can provide a name for the reigster */
    printf("R%d = %04x (BP)\n",BASE_PTR, my_emulator.reg_file[REGISTER][BASE_PTR].word);
    printf("R%d = %04x (LR)\n",LINK_REG, my_emulator.reg_file[REGISTER][LINK_REG].word);
    printf("R%d = %04x (SP)\n",STACK_PTR, my_emulator.reg_file[REGISTER][STACK_PTR].word);
    printf("R%d = %04x (PC)\n",PROG_COUNTER, my_emulator.reg_file[REGISTER][PROG_COUNTER].word);

}
/*
 * @brief This function allows the user to modify the values of the registers in the emulator
 */

void modify_registers()
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
            my_emulator.reg_file[REGISTER][reg_num].word = value;
            printf("Updated Register Value\n");
        }
    }
}
/*
 * @brief This function allows the user to modify the values of the memory in the emulator
 */
void modify_memory_locations()
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
void set_breakpoint()
{
    //temporay breakpoint to check if it is valid before setting it
    int temp_breakpoint;
    printf("Enter a breakpoint (must be >%04x): ", my_emulator.reg_file[REGISTER][PROG_COUNTER].word);
    scanf("%x",&temp_breakpoint);
    //if the breakpoint is not even, make it even for word addressing
    temp_breakpoint = (temp_breakpoint % 2 == 0) ? temp_breakpoint : temp_breakpoint - 1;
    //check if program has passed breakpoint already
    //NOTE future implementations could allow this if desired
    if(temp_breakpoint < my_emulator.reg_file[REGISTER][PROG_COUNTER].word)
    {
        printf("Breakpoint has already been passed\n");
    }
    else
    {
        my_emulator.breakpoint = temp_breakpoint;
        printf("Set breakpoint @ %04x\n", my_emulator.breakpoint);
    }
}
/*
 * @brief This function decodes the instructions in the emulator
    * @param emulator the emulator to decode instructions for
 */
#define USE_FETCH
void decode_instruction(Emulator *emulator)
{
    if(emulator == NULL)
    {
        printf("Emulator is NULL, exiting program, FATAL ERROR\n");
        exit(-1);
    }
    instruction_data current_instruction;
#ifndef USE_FETCH
    do
    {
        //shift starting address right for word addressing since word memory is half the size of byte memory
        current_instruction.word = xm23_memory[I_MEMORY].word[my_emulator.reg_file[REGISTER][PROG_COUNTER].word >> 1];
        if(current_instruction.byte[MSB] < 0x4C && current_instruction.byte[MSB] >= 0x40)
        {
            //opcode is only the MSB for this group
            parse_arithmetic_block(current_instruction, my_emulator.reg_file[REGISTER][PROG_COUNTER].word);
            my_emulator.reg_file[REGISTER][PROG_COUNTER].word+= 2;
        }
        else if (current_instruction.byte[MSB] <= 0x4D && current_instruction.byte[MSB] >= 0x4C)
        {
            parse_reg_manip_block(current_instruction, my_emulator.reg_file[REGISTER][PROG_COUNTER].word);
            my_emulator.reg_file[REGISTER][PROG_COUNTER].word+= 2;
        }
        else if(current_instruction.byte[MSB] >= 0x60 && current_instruction.byte[MSB] <= 0x79)
        {
            parse_reg_init(current_instruction,
                           my_emulator.reg_file[REGISTER][PROG_COUNTER].word);
            my_emulator.reg_file[REGISTER][PROG_COUNTER].word+= 2;
        }
        else
        {
            if(current_instruction.word != 0x0000) {
                printf("%04X: NOT SUPPORTED = %04x\n", my_emulator.reg_file[REGISTER][PROG_COUNTER].word, current_instruction.word);
                my_emulator.reg_file[REGISTER][PROG_COUNTER].word+= 2;

            }
            else {
                printf("%04X: END OF CURRENT INSTRUCTIONS.\n", my_emulator.reg_file[REGISTER][PROG_COUNTER].word);
            }

        }
        execute_instruction(&my_emulator);
        //halt decoding at the breakpoint value, as the next stage will execute_instruction this decoded instruction, so halting here halts before the execution
    }while(current_instruction.word != 0x0000 && my_emulator.reg_file[REGISTER][PROG_COUNTER].word <= my_emulator.breakpoint);
#else
    //shift starting address right for word addressing since word memory is half the size of byte memory
        current_instruction.word = emulator->instruction_register;
        if(current_instruction.byte[MSB] < 0x4C && current_instruction.byte[MSB] >= 0x40)
        {
            //opcode is only the MSB for this group
            parse_arithmetic_block(current_instruction, my_emulator.reg_file[REGISTER][PROG_COUNTER].word);
        }
        else if (current_instruction.byte[MSB] <= 0x4D && current_instruction.byte[MSB] >= 0x4C)
        {
            parse_reg_manip_block(current_instruction, my_emulator.reg_file[REGISTER][PROG_COUNTER].word);
        }
        else if(current_instruction.byte[MSB] >= 0x60 && current_instruction.byte[MSB] <= 0x79)
        {
            parse_reg_init(current_instruction,
                           my_emulator.reg_file[REGISTER][PROG_COUNTER].word);
        }
        else
        {
            if(current_instruction.word != 0x0000) {
                printf("%04X: NOT SUPPORTED = %04x\n", my_emulator.reg_file[REGISTER][PROG_COUNTER].word, current_instruction.word);
            }
            else {
                printf("%04X: NOP.\n", my_emulator.reg_file[REGISTER][PROG_COUNTER].word);
                emulator->opcode = -1;
            }

        }
#endif


    return;
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
void parse_arithmetic_block(instruction_data current_instruction, short starting_addr)
{
    //my_emulator.opcode = current_instruction.word & UPPER_BYTE_MASK;
    my_emulator.opcode = current_instruction.byte[MSB];
    //my_emulator.opcode = current_instruction.byte[MSB];
    my_emulator.operand_bits = current_instruction.byte[LSB];
    //extract the bottom nibble of the opcode
    short instruction_table_index = my_emulator.opcode & LOWER_NIBBLE_MASK;
    if(arithmetic_instruction_table[instruction_table_index].opcode)
    {
        /* get opcode from the table */
        my_emulator.opcode = arithmetic_instruction_table[instruction_table_index].execution_opcode;
        printf("%4X: %s ", starting_addr,
               arithmetic_instruction_table[instruction_table_index].instruction_name);

        unsigned char register_or_constant = my_emulator.operand_bits >> 7,
                        word_or_byte = ((my_emulator.operand_bits >> 6) & B0),
                        source_const = (my_emulator.operand_bits >> 3) & EXTRACT_LOW_THREE_BITS,
                        dest = my_emulator.operand_bits & EXTRACT_LOW_THREE_BITS;
#ifdef FLAG_V1
        my_emulator.my_operands.arithmetic_operands.dest = dest;
        my_emulator.my_operands.arithmetic_operands.register_or_constant = register_or_constant;
        my_emulator.my_operands.arithmetic_operands.word_or_byte = word_or_byte;
        my_emulator.my_operands.arithmetic_operands.source_const = source_const;
#else
        my_emulator.my_operands.dest = dest;
        my_emulator.my_operands.register_or_constant = register_or_constant;
        my_emulator.my_operands.word_or_byte = word_or_byte;
        my_emulator.my_operands.source_const = source_const;
#endif
        if(register_or_constant == CONSTANT)
        {
            printf("R/C = %d, W/B = %d, CON = %d , DEST = R%d\n", register_or_constant, word_or_byte, source_const, dest);
        }
        else
        {
            printf("R/C = %d, W/B = %d, SRC = R%d, DEST = R%d\n", register_or_constant, word_or_byte, source_const, dest);
        }
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
void parse_reg_manip_block(instruction_data current_instruction, short starting_addr)
{
    my_emulator.opcode = current_instruction.word & UPPER_BYTE_MASK;
    my_emulator.operand_bits = current_instruction.byte[LSB];
    short val = (current_instruction.byte[MSB] & LOWER_NIBBLE_MASK);
    if (val == MOV_SWAP)
    {
        unsigned char src_const = (my_emulator.operand_bits >> 3) & EXTRACT_LOW_THREE_BITS, dest = my_emulator.operand_bits & EXTRACT_LOW_THREE_BITS;
        if((current_instruction.byte[LSB] & B7) == B7)
        {
            printf("%04X: SWAP ", starting_addr);
            unsigned char word_or_byte = 0;
            printf("W/B = %d, SRC = R%d , DEST = R%d\n", word_or_byte, src_const, dest);
        }
        else
        {
            printf("%04X: MOV ", starting_addr);
            unsigned char word_or_byte = (current_instruction.byte[LSB] >> 6) & B0;
            printf("W/B = %d, SRC = R%d , DEST = R%d\n", word_or_byte, src_const, dest);
        }
    }
    else if(val == BYTE_MANIP)
    {
        unsigned char comparison_value = (my_emulator.operand_bits >> 3) & EXTRACT_LOW_THREE_BITS;
        unsigned char dest = my_emulator.operand_bits & EXTRACT_LOW_THREE_BITS;
        switch(comparison_value)
        {
            case SRA:
                printf("%04X: SRA, DEST = R%d\n", starting_addr, dest);
                break;
            case RRC:
                printf("%04X: RRC, DEST = R%d\n", starting_addr, dest);
                break;
            case SWPB:
                printf("%04X: SWPB, DEST = R%d\n", starting_addr, dest);
                break;
            case SXT:
                printf("%04X: SXT, DEST = R%d\n", starting_addr, dest);
                break;
            default:
                break;
        }
    }
    return;
}
/*
 * @brief This function parses the move block of instructions
 * @param current_instruction the current instruction to parse
 * @param starting_addr the starting address of the instruction
 */

void parse_reg_init(instruction_data current_instruction, short starting_addr)
{
    //check bits 12 and 11, shift that value to the right to get a value from 0-4, use that to index into the movement_instruction_table
    short table_index = (current_instruction.word >> 11) & EXTRACT_LOW_TWO_BITS;
    my_emulator.opcode = movement_instruction_table[table_index].execution_opcode;
    if(my_emulator.opcode > movh || my_emulator.opcode < movl)
    {
        printf("Invalid opcode\n");
        return;
    }
    else
    {
        printf("%04X: %s ", starting_addr, movement_instruction_table[table_index].instruction_name);
        //extract bytes to be moved from bits 10-3 and destination from bits 2-0
        my_emulator.move_byte = (current_instruction.word >> 3) & 0xFF;
        my_emulator.my_operands.dest = current_instruction.word & EXTRACT_LOW_THREE_BITS;
        printf("BYTE = %02x, DEST = R%d\n", my_emulator.move_byte, my_emulator.my_operands.dest);
    }

}