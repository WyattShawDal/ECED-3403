//
// Created by wyattshaw on 29/05/24.
//

#ifndef ASSIGNMENT1_INSTRUCTION_TABLE_H
#define ASSIGNMENT1_INSTRUCTION_TABLE_H

typedef struct instruction_table_data
{
    short opcode;
    char* instruction_name;
}instruction_table_data;

instruction_table_data arithmetic_instruction_table[] =
    {
        {0x40, "ADD"},
        {0x41, "ADDC"},
        {0x42, "SUB"},
        {0x43, "SUBC"},
        {0x44, "DADD"},
        {0x45, "CMP"},
        {0x46, "XOR"},
        {0x47, "AND"},
        {0x48, "OR"},
        {0x49, "BIT"},
        {0x4A, "BIC"},
        {0x4B, "BIS"},
    };

instruction_table_data movement_instruction_table[] =
        {
                {0, "MOVL"},
                {1, "MOVLZ"},
                {2, "MOVLS"},
                {3, "MOVH"},
        };

#endif //ASSIGNMENT1_INSTRUCTION_TABLE_H
