//
// Created by wyattshaw on 29/05/24.
//

#ifndef ASSIGNMENT1_INSTRUCTION_TABLE_H
#define ASSIGNMENT1_INSTRUCTION_TABLE_H

typedef struct instruction_table_data
{
    short opcode;
    char* instruction_name;
    short execution_opcode;
}instruction_table_data;


instruction_table_data arithmetic_instruction_table[] =
    {
        {0x40, "ADD", add},
        {0x41, "ADDC", addc},
        {0x42, "SUB", sub},
        {0x43, "SUBC", subc},
        {0x44, "DADD", dadd},
        {0x45, "CMP", cmp},
        {0x46, "XOR", xor},
        {0x47, "AND", and},
        {0x48, "OR", or},
        {0x49, "BIT", bit},
        {0x4A, "BIC", bic},
        {0x4B, "BIS", bis},

    };

instruction_table_data movement_instruction_table[] =
        {
                {0, "MOVL", movl},
                {1, "MOVLZ", movlz},
                {2, "MOVLS", movls},
                {3, "MOVH", movh},
        };

#endif //ASSIGNMENT1_INSTRUCTION_TABLE_H
