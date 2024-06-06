//
// Created by wyattshaw on 05/06/24.
//

#include "emulation.h"


#define WORD 0
#define BYTE 1

unsigned char carry_check[2][2][2] = {0, 0, 1, 0, 1, 0, 1, 1};
unsigned char overflow_check[2][2][2] = {0, 1, 0, 0, 0, 0, 1, 0};


void execute(Emulator *emulator)
{

    unsigned char dest = emulator->my_operands.arithmetic_operands.dest;
    unsigned char wb = emulator->my_operands.arithmetic_operands.word_or_byte;
    unsigned char rc = emulator->my_operands.arithmetic_operands.register_or_constant;
    unsigned char sc = emulator->my_operands.arithmetic_operands.source_const;
    switch (emulator->opcode)
    {
        case add:
            if(wb == WORD)
            {
                emulator->reg_file[REGISTER][dest].word +=
                    emulator->reg_file[rc][sc].word;
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator);
            }
            else
            {
                emulator->reg_file[REGISTER][dest].byte[LSB] +=
                emulator->reg_file[rc][sc].byte[LSB];
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB], emulator);
            }

            break;
        case addc:
            break;
        case sub:
            if(wb == WORD)
            {
                emulator->reg_file[REGISTER][dest].word -=
                        emulator->reg_file[rc][sc].word;
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator);
            }
            else
            {
                emulator->reg_file[REGISTER][dest].byte[LSB] -=
                        emulator->reg_file[rc][sc].byte[LSB];
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB], emulator);
            }
            break;
        case subc:
            break;
        case dadd:
            break;
        case cmp:
            if(wb == WORD)
            {
                unsigned short temp;
                temp = emulator->reg_file[REGISTER][dest].word;
                temp -= emulator->reg_file[rc][sc].word;
                update_psw(temp, emulator);
            }
            else
            {
                unsigned char temp;
                temp = emulator->reg_file[REGISTER][dest].word;
                temp -= emulator->reg_file[rc][sc].word;
                update_psw(temp, emulator);
            }
            break;
        case xor:
            if(wb == WORD)
            {
                emulator->reg_file[REGISTER][dest].word ^=
                        emulator->reg_file[rc][sc].word;
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator);
            }
            else
            {
                emulator->reg_file[REGISTER][dest].byte[LSB] ^=
                        emulator->reg_file[rc][sc].byte[LSB];
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB], emulator);
            }
            break;
        case and:
            if(wb == WORD)
            {
                emulator->reg_file[REGISTER][dest].word &=
                        emulator->reg_file[rc][sc].word;
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator);
            }
            else
            {
                emulator->reg_file[REGISTER][dest].byte[LSB] &=
                        emulator->reg_file[rc][sc].byte[LSB];
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB], emulator);
            }
            break;
        case or:
            if(wb == WORD)
            {
                emulator->reg_file[REGISTER][dest].word |=
                        emulator->reg_file[rc][sc].word;
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator);
            }
            else
            {
                emulator->reg_file[REGISTER][dest].byte[LSB] |=
                        emulator->reg_file[rc][sc].byte[LSB];
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB], emulator);
            }
            break;
        case bit:
            break;
        case bic:
            break;
        case bis:
            break;
        case mov:
            break;
        case swap:
            break;
        case sra:
            break;
        case rrc:
            break;
        case swpb:
            break;
        case sxt:
            break;
        case movl:
            break;
        case movlz:
            break;
        case movls:
            break;
        case movh:
            break;

    }
}
#define MSbit 0x8000
void update_psw(unsigned short result, Emulator *emulator)
{
    emulator->psw.zero = result == 0 ? 1 : 0;
    emulator->psw.negative = result & MSbit ? 1 : 0;
    emulator->psw.carry = carry_check[emulator->my_operands.arithmetic_operands.source_const & MSbit]
            [emulator->my_operands.arithmetic_operands.dest & MSbit]
            [result & MSbit];
    emulator->psw.overflow = overflow_check[emulator->my_operands.arithmetic_operands.source_const & MSbit]
    [emulator->my_operands.arithmetic_operands.dest & MSbit]
    [result & MSbit];
}