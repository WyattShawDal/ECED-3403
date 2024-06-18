//
// Created by wyattshaw on 05/06/24.
//

#include "emulation.h"


#define WORD 0
#define BYTE 1

unsigned char carry_check[2][2][2] = {0, 0, 1, 0, 1, 0, 1, 1};
unsigned char overflow_check[2][2][2] = {0, 1, 0, 0, 0, 0, 1, 0};

/*
 * @brief This function executes the instruction based on the current opcode
 * in the emulator
 */
void execute_instruction(Emulator *emulator)
{
//todo change this to use a struct with bit fields.
#ifdef FLAG_V1
    unsigned char dest = emulator->my_operands.arithmetic_operands.dest;
    unsigned char wb = emulator->my_operands.arithmetic_operands.word_or_byte;
    unsigned char rc = emulator->my_operands.arithmetic_operands.register_or_constant;
    unsigned char sc = emulator->my_operands.arithmetic_operands.source_const;
#else
    instruction_data temp_reg;
    unsigned char temp;
    short result;
    unsigned char dest = emulator->my_operands.dest;
    unsigned char wb = emulator->my_operands.word_or_byte;
    unsigned char rc = emulator->my_operands.register_or_constant;
    unsigned char sc = emulator->my_operands.source_const;
#endif
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
        case addc: //add with carry
            if(wb == WORD)
            {
                emulator->reg_file[REGISTER][dest].word +=
                        (emulator->reg_file[rc][sc].word + emulator->psw.carry);
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator);
            }
            else
            {
                emulator->reg_file[REGISTER][dest].byte[LSB] +=
                        (emulator->reg_file[rc][sc].byte[LSB] + emulator->psw.carry);
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB], emulator);
            }
            break;
        case sub:
            if(wb == WORD)
            {
                emulator->reg_file[REGISTER][dest].word +=
                        ~emulator->reg_file[rc][sc].word + 1;
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator);
            }
            else
            {
                emulator->reg_file[REGISTER][dest].byte[LSB] +=
                        ~emulator->reg_file[rc][sc].byte[LSB] + 1;
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB], emulator);
            }
            break;
        case subc: //subtract with carry
            if(wb == WORD)
            {
                emulator->reg_file[REGISTER][dest].word +=
                        emulator->reg_file[rc][sc].word + emulator->psw.carry;
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator);
            }
            else
            {
                emulator->reg_file[REGISTER][dest].byte[LSB] +=
                        emulator->reg_file[rc][sc].byte[LSB] + emulator->psw.carry;
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB],
                           emulator);
            }
            break;
        case dadd: //decimal add
            //todo implement later with nibbles and thigns
            break;
        case cmp:
            if(wb == WORD)
            {
                temp = emulator->reg_file[REGISTER][dest].word;
                temp += (~emulator->reg_file[rc][sc].word + 1);
                update_psw(temp, emulator);
            }
            else
            {
                temp = emulator->reg_file[REGISTER][dest].byte[LSB];
                temp += (~emulator->reg_file[rc][sc].byte[LSB] + 1);
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
        case bit: //bit test
            if(wb == WORD)
            {
                result = emulator->reg_file[REGISTER][dest].word & (1 << emulator->reg_file[rc][sc].word);
                update_psw(result, emulator);
            }
            else
            {
                result = emulator->reg_file[REGISTER][dest].byte[LSB] & (1 << emulator->reg_file[rc][sc].byte[LSB]);
                update_psw(result, emulator);
            }

            break;
        case bic: //bit clear
            if(wb == WORD)
            {
                emulator->reg_file[REGISTER][dest].word &= ~(1 << emulator->reg_file[rc][sc].word);
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator);
            }
            else
            {
                emulator->reg_file[REGISTER][dest].byte[LSB] &= ~(1 << emulator->reg_file[rc][sc].byte[LSB]);
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB], emulator);
            }
            break;
        case bis: //bit set
            if(wb == WORD)
            {
                emulator->reg_file[REGISTER][dest].word |= (1 << emulator->reg_file[rc][sc].word);
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator);
            }
            else
            {
                emulator->reg_file[REGISTER][dest].byte[LSB] |= (1 << emulator->reg_file[rc][sc].byte[LSB]);
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB], emulator);
            }
            break;
        case mov: //move
            if(wb == WORD)
            {
                emulator->reg_file[REGISTER][dest].word = emulator->reg_file[REGISTER][sc].word;
            }
            else
            {
                emulator->reg_file[REGISTER][dest].byte[LSB] = emulator->reg_file[REGISTER][sc].byte[LSB];
            }
            break;
        case swap:
            temp_reg = emulator->reg_file[REGISTER][sc];
            emulator->reg_file[REGISTER][sc] = emulator->reg_file[REGISTER][dest];
            emulator->reg_file[REGISTER][dest] = temp_reg;
            break;
        case sra: //shift right arithmetic
            /*todo investigate better ways to do the rotations */

            ((emulator->reg_file[REGISTER][dest].word & 0x0001) == 1) ? emulator->psw.carry = 1 : 0;
            if(wb == WORD)
            {
                /* save the value of the MSbit into temp */
                emulator->reg_file[REGISTER][dest].word & 0x8000 ? temp &= 0x8000 : 0x0000;
                /* perform the right shift one bit */
                emulator->reg_file[REGISTER][dest].word >>= 1;
                /* restore the MSbit */
                emulator->reg_file[REGISTER][dest].word |= temp;
            }
            else
            {
                /*todo check if we should be checking the msbit of low byte or
                 * whole byte*/

                /* save the value of the MSbit into temp */
                emulator->reg_file[REGISTER][dest].byte[LSB] & 0x80 ? temp &= 0x80 : 0;
                /* perform the right shift one bit */
                emulator->reg_file[REGISTER][dest].byte[LSB] >>= 1;
                /* restore the MSbit */
                emulator->reg_file[REGISTER][dest].byte[LSB] |= temp;
            }
            break;
        case rrc: //rotate right through carry
            if(wb == WORD)
            {
                //does this work? maybe, shift right one and then or with the LSbit shifted to the MSbit
                emulator->reg_file[REGISTER][dest].word = (emulator->reg_file[REGISTER][dest].word >> 1) |
                        (emulator->reg_file[REGISTER][dest].word << 15);
            }
            else
            {
                emulator->reg_file[REGISTER][dest].byte[LSB] = (emulator->reg_file[REGISTER][dest].byte[LSB] >> 1) |
                        (emulator->reg_file[REGISTER][dest].byte[LSB] << 7);
            }
            break;
        case swpb: //swap bytes
            temp_reg.byte[MSB] = emulator->reg_file[REGISTER][dest].byte[MSB];
            emulator->reg_file[REGISTER][dest].byte[MSB] = emulator->reg_file[REGISTER][dest].byte[LSB];
            emulator->reg_file[REGISTER][dest].byte[LSB] = temp_reg.byte[MSB];
            break;
        case sxt:
            emulator->reg_file[REGISTER][dest].byte[LSB] & 0x80 ? emulator->reg_file[REGISTER][dest].byte[MSB] |= 0xFF : 0;
            break;
        case movl:
            emulator->reg_file[REGISTER][dest].byte[LSB] = emulator->move_byte;
            break;
        case movlz:
            emulator->reg_file[REGISTER][dest].byte[LSB] = emulator->move_byte;
            emulator->reg_file[REGISTER][dest].byte[MSB] = 0x00;
            break;
        case movls:
            emulator->reg_file[REGISTER][dest].byte[LSB] = emulator->move_byte;
            emulator->reg_file[REGISTER][dest].byte[MSB] = 0xFF;

            break;
        case movh:
            emulator->reg_file[REGISTER][dest].byte[MSB] = emulator->move_byte;
            break;
        case -1:
            printf("NOP Executed\n");
            break;
        default:
            printf("Invalid Opcode\n");
            break;

    }
}
#define MSbit 0x8000
/*
 * @brief This function updates the program status word based on the result of
 * executed operations, not all operations will cause this function to be called
 */
void update_psw(unsigned short result, Emulator *emulator)
{
    emulator->psw.zero = result == 0 ? 1 : 0;
    emulator->psw.negative = result & MSbit ? 1 : 0;
#ifdef FLAG_V1
    emulator->psw.carry = carry_check[emulator->my_operands.arithmetic_operands.source_const & MSbit]
            [emulator->my_operands.arithmetic_operands.dest & MSbit]
            [result & MSbit];
    emulator->psw.overflow = overflow_check[emulator->my_operands.arithmetic_operands.source_const & MSbit]
    [emulator->my_operands.arithmetic_operands.dest & MSbit]
    [result & MSbit];
#else
    //todo change source/const and dest to registers and not variables
    emulator->psw.carry = carry_check[emulator->my_operands.source_const & MSbit]
    [emulator->my_operands.dest & MSbit]
    [result & MSbit];
    emulator->psw.overflow = overflow_check[emulator->my_operands.source_const & MSbit]
    [emulator->my_operands.dest & MSbit]
    [result & MSbit];
#endif

}