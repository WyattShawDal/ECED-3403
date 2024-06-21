/*
 * File Name: execution.c
 * Date June 20 2024
 * Written By: Wyatt Shaw
 * Module Info: This module implements the execution of the instructions in the emulator as well as the
 * updating of the program status word
 */
#include "emulation.h"


#define WORD 0
#define BYTE 1

unsigned char carry_check[2][2][2] = {0, 0, 1, 0, 1, 0, 1, 1};
unsigned char overflow_check[2][2][2] = {0, 1, 0, 0, 0, 0, 1, 0};

/*
 * @brief This function executes the instruction based on the current opcode
 * in the emulator
 */
void execute_instruction(Emulator *emulator) {

    instruction_data temp_reg;
    unsigned short temp;
    unsigned short old_dest;
    unsigned short result;
    /* local variables copies of operands are made to improve readability */
    unsigned char dest = emulator->my_operands.dest;
    unsigned char wb = emulator->my_operands.word_or_byte;
    unsigned char rc = emulator->my_operands.register_or_constant;
    unsigned char sc = emulator->my_operands.source_const;
    if (wb == WORD) {
        old_dest = emulator->reg_file[REGISTER][dest].word;
    } else {
        old_dest = emulator->reg_file[REGISTER][dest].byte[LSB];
    }
    switch (emulator->opcode) {
        case add:
            printf("ADD Executed\n");
            if (wb == WORD) {
                emulator->reg_file[REGISTER][dest].word += emulator->reg_file[rc][sc].word;

                update_psw(emulator->reg_file[REGISTER][dest].word, emulator,
                           old_dest, emulator->reg_file[rc][sc].word);
            } else {
                emulator->reg_file[REGISTER][dest].byte[LSB] += emulator->reg_file[rc][sc].byte[LSB];

                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB],
                           emulator, old_dest, emulator->reg_file[rc][sc].byte[LSB]);
            }

            break;
        case addc: //add with carry
            printf("ADDC Executed\n");
            if (wb == WORD) {
                emulator->reg_file[REGISTER][dest].word +=
                        (emulator->reg_file[rc][sc].word + emulator->psw.carry);
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator,
                           old_dest, emulator->reg_file[rc][sc].byte[LSB]);
            } else {
                emulator->reg_file[REGISTER][dest].byte[LSB] +=
                        (emulator->reg_file[rc][sc].byte[LSB] + emulator->psw.carry);
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB],
                           emulator, old_dest, emulator->reg_file[rc][sc].byte[LSB]);
            }
            break;
        case sub:
            printf("SUB Executed\n");
            if (wb == WORD) {
                emulator->reg_file[REGISTER][dest].word +=
                        ~emulator->reg_file[rc][sc].word + 1;
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator,
                           old_dest, ~emulator->reg_file[rc][sc].word);
            } else {
                emulator->reg_file[REGISTER][dest].byte[LSB] +=
                        ~emulator->reg_file[rc][sc].byte[LSB] + 1;
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB],
                           emulator, old_dest, ~emulator->reg_file[rc][sc].byte[LSB]);
            }
            break;
        case subc: //subtract with carry
            printf("SUBC Executed\n");
            if (wb == WORD) {
                emulator->reg_file[REGISTER][dest].word +=
                        emulator->reg_file[rc][sc].word + emulator->psw.carry;
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator,
                           old_dest, ~emulator->reg_file[rc][sc].word);
            } else {
                emulator->reg_file[REGISTER][dest].byte[LSB] +=
                        emulator->reg_file[rc][sc].byte[LSB] + emulator->psw.carry;
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB],
                           emulator, old_dest, ~emulator->reg_file[rc][sc].byte[LSB]);
            }
            break;
        case dadd: //decimal add
            printf("DADD Executed\n");

            bcd_addition(emulator);
            break;
        case cmp:
            printf("CMP Executed\n");
            if (wb == WORD) {
                temp = emulator->reg_file[REGISTER][dest].word;
                temp += (~emulator->reg_file[rc][sc].word + 1);
                update_psw(temp, emulator, old_dest, ~emulator->reg_file[rc][sc].word);
            } else {
                temp = emulator->reg_file[REGISTER][dest].byte[LSB];
                temp += (~emulator->reg_file[rc][sc].byte[LSB] + 1);
                update_psw(temp, emulator, old_dest, ~emulator->reg_file[rc][sc].byte[LSB]);
            }
            break;
        case xor:
            printf("XOR Executed\n");

            if (wb == WORD) {
                emulator->reg_file[REGISTER][dest].word ^=
                        emulator->reg_file[rc][sc].word;
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator,
                           old_dest, emulator->reg_file[rc][sc].word);
            } else {
                emulator->reg_file[REGISTER][dest].byte[LSB] ^=
                        emulator->reg_file[rc][sc].byte[LSB];
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB],
                           emulator, old_dest, emulator->reg_file[rc][sc].byte[LSB]);
            }
            break;
        case and:
            printf("AND Executed\n");

            if (wb == WORD) {
                emulator->reg_file[REGISTER][dest].word &=
                        emulator->reg_file[rc][sc].word;
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator,
                           old_dest, emulator->reg_file[rc][sc].word);
            } else {
                emulator->reg_file[REGISTER][dest].byte[LSB] &=
                        emulator->reg_file[rc][sc].byte[LSB];
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB],
                           emulator, old_dest, emulator->reg_file[rc][sc].byte[LSB]);
            }
            break;
        case or:
            printf("OR Executed\n");

            if (wb == WORD) {
                emulator->reg_file[REGISTER][dest].word |=
                        emulator->reg_file[rc][sc].word;
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator,
                           old_dest, emulator->reg_file[rc][sc].word);
            } else {
                emulator->reg_file[REGISTER][dest].byte[LSB] |=
                        emulator->reg_file[rc][sc].byte[LSB];
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB],
                           emulator, old_dest, emulator->reg_file[rc][sc].byte[LSB]);
            }
            break;
        case bit: //bit test
            printf("BIT Executed\n");

            if (wb == WORD) {
                result = emulator->reg_file[REGISTER][dest].word & (1 << emulator->reg_file[rc][sc].word);
                update_psw(result, emulator, old_dest, emulator->reg_file[rc][sc].word);
            } else {
                result = emulator->reg_file[REGISTER][dest].byte[LSB] & (1 << emulator->reg_file[rc][sc].byte[LSB]);
                update_psw(result, emulator, old_dest, emulator->reg_file[rc][sc].byte[LSB]);
            }

            break;
        case bic: //bit clear
            printf("BIC Executed\n");

            if (wb == WORD) {
                emulator->reg_file[REGISTER][dest].word &= ~(1 << emulator->reg_file[rc][sc].word);
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator,
                           old_dest, emulator->reg_file[rc][sc].word);
            } else {
                emulator->reg_file[REGISTER][dest].byte[LSB] &= ~(1 << emulator->reg_file[rc][sc].byte[LSB]);
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB],
                           emulator, old_dest, emulator->reg_file[rc][sc].byte[LSB]);
            }
            break;
        case bis: //bit set
            printf("BIS Executed\n");

            if (wb == WORD) {
                emulator->reg_file[REGISTER][dest].word |= (1 << emulator->reg_file[rc][sc].word);
                update_psw(emulator->reg_file[REGISTER][dest].word, emulator,
                           old_dest, emulator->reg_file[rc][sc].word);
            } else {
                emulator->reg_file[REGISTER][dest].byte[LSB] |= (1 << emulator->reg_file[rc][sc].byte[LSB]);
                update_psw(emulator->reg_file[REGISTER][dest].byte[LSB],
                           emulator, old_dest, emulator->reg_file[rc][sc].byte[LSB]);
            }
            break;
        case mov: //move
            printf("MOV Executed\n");

            if (wb == WORD) {
                emulator->reg_file[REGISTER][dest].word = emulator->reg_file[REGISTER][sc].word;
            } else {
                emulator->reg_file[REGISTER][dest].byte[LSB] = emulator->reg_file[REGISTER][sc].byte[LSB];
            }
            break;
        case swap:
            printf("SWAP Executed\n");

            temp_reg = emulator->reg_file[REGISTER][sc];
            emulator->reg_file[REGISTER][sc] = emulator->reg_file[REGISTER][dest];
            emulator->reg_file[REGISTER][dest] = temp_reg;
            break;
        case sra: //shift right arithmetic
            printf("SRA Executed\n");

            ((emulator->reg_file[REGISTER][dest].word & 0x0001) == 1) ? emulator->psw.carry = 1 : 0;
            if (wb == WORD) {
                /* save the value of the MSbit into temp */
                (emulator->reg_file[REGISTER][dest].word & WORD_MSb) == WORD_MSb ? temp = WORD_MSb : 0x0000;
                /* perform the right shift one bit */
                emulator->reg_file[REGISTER][dest].word >>= 1;
                /* restore the MSbit */
                emulator->reg_file[REGISTER][dest].word |= temp;
            } else {

                /* save the value of the MSbit into temp */
                emulator->reg_file[REGISTER][dest].byte[LSB] & BYTE_MSb ? temp = BYTE_MSb : 0;
                /* perform the right shift one bit */
                emulator->reg_file[REGISTER][dest].byte[LSB] >>= 1;
                /* restore the MSbit */
                emulator->reg_file[REGISTER][dest].byte[LSB] |= temp;
            }
            break;
        case rrc: //rotate right through carry
            printf("RRC Executed\n");

            //set a variable equal to the carry
            temp = emulator->psw.carry;

            //if the lsbit of the number is 1 set the carry
            ((emulator->reg_file[REGISTER][dest].word & 0x0001) == 1) ? emulator->psw.carry = 1 : 0;

            //rotate the number right one

            //set the msbit equal to variable
            if (wb == WORD) {
                //todo define 7 and 15
                //does this work? maybe, shift right one and then or with the LSbit shifted to the MSbit
                // msbit moves to lsbit which covers carry from the roation, and the extra or is from carry from diff op?
                emulator->reg_file[REGISTER][dest].word =
                        (emulator->reg_file[REGISTER][dest].word >> 1) | (temp << WORD_SHIFT);
            } else {
                emulator->reg_file[REGISTER][dest].byte[LSB] =
                        (emulator->reg_file[REGISTER][dest].byte[LSB] >> 1) | (temp << BYTE_SHIFT);
            }
            break;
        case swpb: //swap bytes
            printf("SWPB Executed\n");
            temp_reg.byte[MSB] = emulator->reg_file[REGISTER][dest].byte[MSB];
            emulator->reg_file[REGISTER][dest].byte[MSB] = emulator->reg_file[REGISTER][dest].byte[LSB];
            emulator->reg_file[REGISTER][dest].byte[LSB] = temp_reg.byte[MSB];
            break;
        case sxt: //sign extend
            printf("SXT Executed\n");
            emulator->reg_file[REGISTER][dest].byte[LSB] & BYTE_MSb ? emulator->reg_file[REGISTER][dest].byte[MSB] |= 0xFF: 0;
            break;
        case movl:
            printf("MOVL Executed\n");
            emulator->reg_file[REGISTER][dest].byte[LSB] = emulator->move_byte;
            break;
        case movlz:
            printf("MOVLZ Executed\n");

            emulator->reg_file[REGISTER][dest].byte[LSB] = emulator->move_byte;
            emulator->reg_file[REGISTER][dest].byte[MSB] = 0x00;
            break;
        case movls:
            printf("MOVLS Executed\n");

            emulator->reg_file[REGISTER][dest].byte[LSB] = emulator->move_byte;
            emulator->reg_file[REGISTER][dest].byte[MSB] = 0xFF;
            break;
        case movh:
            printf("MOVH Executed\n");

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

/*
 *
 * @brief This function updates the program status word based on the result of
 * executed operations, not all operations will cause this function to be called
 * @param result the result of the previous operation
 * @param emulator the emulator struct for updating the psw
 * @param old_dest the value of the destination register before the operation
 * @param source the value of the source register or constant used in the operation, one's complement is passed
 * for subtraction operations (sub, subc, cmp)
 */

void update_psw(unsigned short result, Emulator *emulator, unsigned short old_dest, unsigned short source) {
    unsigned short ms_bit;
    unsigned char shift_size;
    if (emulator->my_operands.word_or_byte == WORD) {
        ms_bit = WORD_MSb;
        shift_size = WORD_SHIFT;
        emulator->psw.negative = result & ms_bit ? 1 : 0;
    } else {
        ms_bit = BYTE_MSb;
        shift_size = BYTE_SHIFT;
        emulator->psw.negative = result & ms_bit ? 1 : 0;

    }

    emulator->psw.zero = result == 0 ? 1 : 0;
    emulator->psw.carry = carry_check[(source & ms_bit) >> shift_size][(old_dest & ms_bit) >> shift_size][(result & ms_bit) >> shift_size];
    emulator->psw.overflow = overflow_check[(source & ms_bit) >> shift_size][(old_dest & ms_bit) >> shift_size][(result & ms_bit) >> shift_size];

}

#define RESET_HEX 10
/*
 * @brief This function performs a binary coded decimal addition on the source and destination registers
 *
 *
 */
void bcd_addition(Emulator *emulator) {
    //set operand values to local variables
    unsigned char dest = emulator->my_operands.dest;
    unsigned char wb = emulator->my_operands.word_or_byte;
    unsigned char rc = emulator->my_operands.register_or_constant;
    unsigned char sc = emulator->my_operands.source_const;


    //create nibble structs to hold the bcd values
    word_nibbles source_bcd;
    word_nibbles dest_bcd;
    word_nibbles result_bcd;

    //set the values of the nibbles to the values of the registers
    dest_bcd.word = emulator->reg_file[REGISTER][dest].word;
    source_bcd.word = emulator->reg_file[rc][sc].word;

    //check if the values are greater than 9
    if (source_bcd.word > 0x09) {
        //if the values from the constants table are either 16 or 32 then convert them to bcd
        switch (source_bcd.word) {
            case 16:
                source_bcd.nibbles.nib1 = 1;
                source_bcd.nibbles.nib0 = 6;
                break;
            case 32:
                source_bcd.nibbles.nib1 = 3;
                source_bcd.nibbles.nib0 = 2;
                break;
            //default case is for the case where the value is not 16 or 32 which means found a hex value that is not
            //a bcd value for us to handle, programmer is responsible
            default:
                break;
        }
    }
    //work our way up from lower nibble to highest nibble, if the value exceeds 9 then subtract 10 and add 1 to the next nibble
    result_bcd.nibbles.nib0 += dest_bcd.nibbles.nib0 + source_bcd.nibbles.nib0;
    if (result_bcd.nibbles.nib0 > 9) {
        result_bcd.nibbles.nib0 -= RESET_HEX;
        result_bcd.nibbles.nib1++;
    }
    result_bcd.nibbles.nib1 += dest_bcd.nibbles.nib1 + source_bcd.nibbles.nib1;
    if (result_bcd.nibbles.nib1 > 9) {
        result_bcd.nibbles.nib1 -= RESET_HEX;
        //if dadd.b set the carry flag and don't increment the next nibble, else increment the next nibble
        wb == WORD ? result_bcd.nibbles.nib2++ : (emulator->psw.carry = 1);
    }
    if (wb == WORD) {
        result_bcd.nibbles.nib2 += dest_bcd.nibbles.nib2 + source_bcd.nibbles.nib2;
        if (result_bcd.nibbles.nib2 > 9) {
            result_bcd.nibbles.nib2 -= RESET_HEX;
            result_bcd.nibbles.nib3++;
        }
        result_bcd.nibbles.nib3 += dest_bcd.nibbles.nib3 + source_bcd.nibbles.nib3;
        if (result_bcd.nibbles.nib3 > 9) {
            result_bcd.nibbles.nib3 -= RESET_HEX;
            emulator->psw.carry = 1;
        } else {
            emulator->psw.carry = 0;
        }
    }
    //set the result of the bcd addition to the destination register
    emulator->reg_file[REGISTER][dest].word = result_bcd.word;

}
