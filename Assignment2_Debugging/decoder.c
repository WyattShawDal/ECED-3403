//
// Created by wyatt on 2024-05-29.
//
#include "decoder.h"
#include "instruction_table.h"

extern Emulator my_emulator;

#define MSB 1
#define LSB 0

void debugger_menu()
{
    printf("Entered Debugger Menu\n");
    char command;
    do {
        printf("Print Decode (D), Print Register(R), Modify Register Values (T), Modify Memory Value (U),Set Breakpoint Value (Y), Quit (Q)\n");
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
                decode_instruction();
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

#define REGISTER 0
#define CONSTANT 1
void print_registers()
{
    for (int i = 0; i < REGFILE; ++i)
    {
        printf("R%d = %04x\n", i, my_emulator.reg_file[REGISTER][i]);
    }
}
void modify_registers()
{
    unsigned short reg_num = 0;
    unsigned int value= 0;
    printf("Hello! Enter Register Number (Decimal): ");
    scanf("%hd", &reg_num);
    if(reg_num < 0 || reg_num > 7)
    {
        printf("Bad Reg Num\n");
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
            my_emulator.reg_file[REGISTER][reg_num] = value;
            printf("Updated Register Value\n");
        }
    }
}
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
    mem_type = mem_type == 'i' ? I_MEMORY : D_MEMORY;

    printf("Enter address to modify (in hex): ");
    scanf("%x", &address);
    if(address > (WORD_MEMORY_SIZE) || address < 0)
    {
        printf("Invalid address entered");
        return;
    }
    printf("Enter word to set (0-FFFF): ");
    scanf("%hx", &value);
    loader_memory[mem_type].word[address>>1] = value;
}
void set_breakpoint()
{
    int temp_breakpoint;
    printf("Enter a breakpoint (must be >%04x): ", my_emulator.program_counter);
    scanf("%x",&temp_breakpoint);
    temp_breakpoint = (temp_breakpoint % 2 == 0) ? temp_breakpoint : temp_breakpoint - 1;
    if(temp_breakpoint < my_emulator.program_counter)
    {
        printf("Breakpoint has already been passed\n");
    }
    else
    {
        my_emulator.breakpoint = temp_breakpoint;
        printf("Set breakpoint @ %04x\n", my_emulator.breakpoint);
    }
}
#define PROG_COUNTER 7
void decode_instruction()
{
    instruction_data current_instruction;
    //set starting address to the starting address of the instruction memory
    current_instruction.word = 0x0001;
    //not at end of instruction memory
    do
    {
        //shift starting address right for word addressing since word memory is half the size of byte memory
        current_instruction.word = loader_memory[I_MEMORY].word[my_emulator.program_counter >> 1];
        if(current_instruction.byte[MSB] < 0x4C && current_instruction.byte[MSB] >= 0x40)
        {
            //opcode is only the MSB for this group
            parse_arithmetic_block(current_instruction, my_emulator.program_counter);
        }
        else if (current_instruction.byte[MSB] <= 0x4D && current_instruction.byte[MSB] >= 0x4C)
        {
            parse_reg_manip_block(current_instruction, my_emulator.program_counter);
        }
        else if(current_instruction.byte[MSB] >= 0x60 && current_instruction.byte[MSB] <= 0x79)
        {
            parse_move_block(current_instruction, my_emulator.program_counter);
        }
        else
        {
            printf("%04X: NOT SUPPORTED = %04x\n", my_emulator.program_counter, current_instruction.word);
        }
        my_emulator.program_counter+= 2;
        my_emulator.reg_file[REGISTER][PROG_COUNTER] = my_emulator.program_counter;
    }while(current_instruction.word != 0x0000 && my_emulator.program_counter < my_emulator.breakpoint);
    my_emulator.program_counter = my_emulator.starting_address;
    return;
}

#define UPPER_BYTE_MASK 0xFF00
#define LOWER_NIBBLE_MASK 0x0F
#define CONSTANT 1

void parse_arithmetic_block(instruction_data current_instruction, short starting_addr)
{
    //my_emulator.opcode = current_instruction.word & UPPER_BYTE_MASK;
    unsigned char extracted_opcode = current_instruction.byte[MSB];
    my_emulator.opcode = current_instruction.byte[MSB];
    my_emulator.operands = current_instruction.byte[LSB];
    //extract the bottom nibble of the opcode
    short instruction_table_index = my_emulator.opcode & LOWER_NIBBLE_MASK;
    if(arithmetic_instruction_table[instruction_table_index].opcode)
    {
        printf("%4X: %s ", starting_addr,
               arithmetic_instruction_table[instruction_table_index].instruction_name);

        unsigned char register_or_constant = my_emulator.operands >> 7, word_or_byte = ((my_emulator.operands >> 6) & B0),
        source_const = (my_emulator.operands >> 3) & EXTRACT_LOW_THREE_BITS, dest = my_emulator.operands & EXTRACT_LOW_THREE_BITS;
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

void parse_reg_manip_block(instruction_data current_instruction, short starting_addr)
{
    my_emulator.opcode = current_instruction.word & UPPER_BYTE_MASK;
    my_emulator.operands = current_instruction.byte[LSB];
    short val = (current_instruction.byte[MSB] & LOWER_NIBBLE_MASK);
    if (val == MOV_SWAP)
    {
        unsigned char src_const = (my_emulator.operands >> 3) & EXTRACT_LOW_THREE_BITS, dest = my_emulator.operands & EXTRACT_LOW_THREE_BITS;
        if((current_instruction.byte[LSB] & B7) == B7)
        {
            printf("%04X: SWAP", starting_addr);
            unsigned char word_or_byte = 0;
            printf("W/B = %d, SRC = %d , DEST = R%d\n", word_or_byte, src_const, dest);
        }
        else
        {
            printf("%04X: MOV ", starting_addr);
            unsigned char word_or_byte = (current_instruction.byte[LSB] >> 6) & B0;
            printf("W/B = %d, SRC = %d , DEST = R%d\n", word_or_byte, src_const, dest);
        }
    }
    else if(val == BYTE_MANIP)
    {
        unsigned char comparison_value = (my_emulator.operands >> 3) & EXTRACT_LOW_THREE_BITS;
        unsigned char dest = my_emulator.operands & EXTRACT_LOW_THREE_BITS;
        switch(comparison_value)
        {
            case SRA:
                printf("%04X: SRA, DEST = %d", starting_addr, dest);
                break;
            case RRC:
                printf("%04X: RRC, DEST = %d", starting_addr, dest);
                break;
            case SWPB:
                printf("%04X: SWPB, DEST = %d", starting_addr, dest);
                break;
            case SXT:
                printf("%04X: SXT, DEST = %d", starting_addr, dest);
                break;
            default:
                break;
        }
    }
    return;
}

#define MOVE_INSTR_MASK 0x1800
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
        printf("%04X: %s ", starting_addr, movement_instruction_table[table_index].instruction_name);
        //extract bytes to be moved from bits 10-3 and destination from bits 2-0
        unsigned char bits_to_move = (current_instruction.word >> 3) & 0xFF;
        unsigned char dest = current_instruction.word & EXTRACT_LOW_THREE_BITS;
        printf("BYTE = %02x, DEST = R%d\n", bits_to_move, dest);
    }

}