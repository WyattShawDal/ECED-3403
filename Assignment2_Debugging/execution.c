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


void execute_1(Emulator *emulator)
{
    memory_controller(emulator);
    if (emulator->opcode == ld || emulator->opcode == ldr)
    {
        emulator->reg_file[REGISTER][emulator->inst_operands.dest].word = emulator->d_control.DMBR;
    }
}
short calc_index_adjustment(Emulator *emulator)
{
    short index_adjustment;
    if(emulator->inst_operands.inc != 0)
    {
        index_adjustment = (emulator->inst_operands.word_or_byte == 0) ? 2 : 1;
    }
    else if(emulator->inst_operands.dec != 0)
    {
        index_adjustment = (emulator->inst_operands.word_or_byte == 0) ? -2 : -1;
    }
    else
    {
        index_adjustment = 0;
    }
    return index_adjustment;
}
void execute_0(Emulator *emulator) {
    switch (emulator->opcode) {
        case add ... bis:
            execute_arithmetic(emulator);
            break;
        case mov ... swap: // move
            execute_mov_swap(emulator);
        case sra ... sxt:
            execute_reg_manip(emulator);
        case ld ... str:
            execute_load_store(emulator);
            break;
        case movl ... movh:
            execute_chg_reg(emulator);
        case setcc:
            emulator->psw.word |= emulator->cpu_ops.byte;
            break;
        case clrcc:
            emulator->psw.word &= ~emulator->cpu_ops.byte;
            break;
        case -1:
            break;
        default:
//            printf("Invalid Opcode\n");
            break;
    }
}

void execute_mov_swap(Emulator *emulator) {
    unsigned char dest = emulator->inst_operands.dest;
    unsigned char sc = emulator->inst_operands.source_const;
    unsigned short destination = emulator->reg_file[REGISTER][dest].word; //ld/st only use register
    unsigned short source = emulator->reg_file[REGISTER][sc].word; //ld/st only use register
    instruction_data temp_reg;
    switch(emulator->opcode)
    {
        case mov:
            destination = source;
            break;
        case swap:
            temp_reg = emulator->reg_file[REGISTER][sc];
            emulator->reg_file[REGISTER][sc] = emulator->reg_file[REGISTER][dest];
            emulator->reg_file[REGISTER][dest] = temp_reg;
            break;

    }
    //update reg file
    emulator->reg_file[REGISTER][sc].word = source;
    emulator->reg_file[REGISTER][dest].word = destination;

}

void execute_reg_manip(Emulator *emulator) {
    instruction_data temp_reg;
    unsigned short temp;
    unsigned char wb = emulator->inst_operands.word_or_byte;
    unsigned char dest = emulator->inst_operands.dest;
    unsigned short destination = emulator->reg_file[REGISTER][dest].word; //ld/st only use register

    switch(emulator->opcode)
    {
        case sra: // shift right arithmetic
            ((destination & 0x0001) == 1) ? emulator->psw.bits.carry = 1 : 0;
            if (wb == WORD) {
                (destination & WORD_MSb) == WORD_MSb ? temp = WORD_MSb : 0x0000; //keep the msbit if set
                destination >>= 1;
                destination |= temp;
            } else {
                destination & BYTE_MSb ? temp = BYTE_MSb : 0;
                destination >>= 1;
                destination |= temp;
            }

            break;
        case rrc: // rotate right through carry
            //store current carry
            temp = emulator->psw.bits.carry;
            //update the carry according to our LSB
            emulator->psw.bits.carry = destination & 0x0001;
            //shift destination register
            destination = (destination >> 1);
            //shift carry into MSBit
            wb == WORD ? destination |= temp << WORD_SHIFT : (destination |= temp << BYTE_SHIFT);
            break;
        case swpb: // swap bytes
            temp_reg.byte[MSB] = destination >> 8;
            destination = (destination << 8) | temp_reg.byte[MSB];
            break;
        case sxt: // sign extend
            (destination & BYTE_MSb) == BYTE_MSb ? destination |= 0xFF00 : 0;
            break;
    }
    if (wb == WORD) {
        emulator->reg_file[REGISTER][dest].word = destination;
    } else {
        emulator->reg_file[REGISTER][dest].byte[LSB] = destination;
    }
}

void execute_load_store(Emulator *emulator) {

    unsigned char sc = emulator->inst_operands.source_const;
    unsigned char wb = emulator->inst_operands.word_or_byte;
    unsigned char dest = emulator->inst_operands.dest;

    short index_adjustment = calc_index_adjustment(emulator);
    unsigned short source = emulator->reg_file[REGISTER][sc].word; //ld/st only use register
    unsigned short destination = emulator->reg_file[REGISTER][dest].word; //ld/st only use register
    switch(emulator->opcode)
    {
        case ld:
                if ((emulator->inst_operands.prpo + emulator->inst_operands.inc + emulator->inst_operands.dec) == 0) {
                    emulator->d_control.DMAR = source; //EA <-- Source, DMAR <-- EA
                } else if (emulator->inst_operands.prpo == 0) //post
                {
                    emulator->d_control.DMAR = source;
                    source += index_adjustment;
                } else //pre
                {
                    source += index_adjustment;
                    emulator->d_control.DMAR = source;
                }
                emulator->xCTRL = D_READ + wb;
                break;
            case st:
                //CASE UTILIZES FALLTHROUGH INTO STR
                if ((emulator->inst_operands.prpo + emulator->inst_operands.inc + emulator->inst_operands.dec) == 0) {
                    emulator->d_control.DMAR = destination;
                } else if (emulator->inst_operands.prpo == 0) //post
                {
                    emulator->d_control.DMAR = destination;
                    destination += index_adjustment;
                } else //pre
                {
                    destination += index_adjustment;
                    emulator->d_control.DMAR = destination;
                }
                //NOTE FALLTHROUGH OCCURS HERE INTENTIONALLY
            case str:
                //CONTROL CAN ENTER HERE WITH ST
                if (emulator->opcode != st) {
                    emulator->d_control.DMAR = destination + emulator->offset;
                }
                //THIS EXECUTES WITH AN ST OR STR
                emulator->d_control.DMBR = source;
                emulator->xCTRL = D_WRITE + wb;
                break;
            case ldr:
                emulator->d_control.DMAR = source + emulator->offset;
                emulator->xCTRL = D_READ + wb;
        break;
    }
    //update source and dest in case of pre/post operation
    if (wb == WORD) {
        emulator->reg_file[REGISTER][sc].word = source;
        emulator->reg_file[REGISTER][dest].word = destination;
    } else {
        emulator->reg_file[REGISTER][sc].byte[LSB] = source;
        emulator->reg_file[REGISTER][dest].byte[LSB] = destination;
    }

}

void execute_chg_reg(Emulator *emulator) {
    unsigned char dest = emulator->inst_operands.dest;
    switch (emulator->opcode) {
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
    }

}

void execute_arithmetic(Emulator *emulator) {
    unsigned short temp;
    unsigned short old_dest;
    unsigned short destination;
    unsigned short source;
    unsigned char dest = emulator->inst_operands.dest;
    unsigned char wb = emulator->inst_operands.word_or_byte;
    unsigned char rc = emulator->inst_operands.register_or_constant;
    unsigned char sc = emulator->inst_operands.source_const;

    if (wb == WORD) {
        //destination before any operations
        old_dest = emulator->reg_file[REGISTER][dest].word;
        destination = emulator->reg_file[REGISTER][dest].word;
        source = emulator->reg_file[rc][sc].word;
    } else {
        //destination before any operations
        old_dest = emulator->reg_file[REGISTER][dest].byte[LSB];
        destination = emulator->reg_file[REGISTER][dest].byte[LSB];
        source = emulator->reg_file[rc][sc].byte[LSB];
    }
    switch (emulator->opcode) {
        case add:
            destination += source;
            update_psw(destination, emulator, old_dest, source);
            break;
        case addc: // add with carry
            destination += source + emulator->psw.bits.carry;
            update_psw(destination, emulator, old_dest, source);
            break;
        case sub:
            destination += ~source + 1;
            update_psw(destination, emulator, old_dest, ~source);

            break;
        case subc: // subtract with carry
            destination += ~source + 1 + emulator->psw.bits.carry;
            update_psw(destination, emulator, old_dest, ~source);
            break;
        case dadd: // decimal add
            bcd_addition(emulator);
            destination = emulator->reg_file[REGISTER][dest].word;
            break;
        case cmp:
            temp = destination + (~source + 1);
            update_psw(temp, emulator, old_dest, ~source);
            break;
        case xor:
            destination ^= source;
            update_psw(destination, emulator, old_dest, source);
            break;
        case and:
            destination &= source;
            update_psw(destination, emulator, old_dest, source);
            break;
        case or:
            destination |= source;
            update_psw(destination, emulator, old_dest, source);
            break;
        case bit: // bit test
            temp = destination & (1 << source);
            update_psw(temp, emulator, old_dest, source);
            break;
        case bic: // bit clear
            destination &= ~(1 << source);
            update_psw(destination, emulator, old_dest, source);
            break;
        case bis: // bit set
            destination |= (1 << source);
            update_psw(destination, emulator, old_dest, source);
            break;
        default:
            break;
    }
    //update registers
    //todo source doesn't change i don't think so should be fine to remove source from here
    if (wb == WORD) {
        emulator->reg_file[rc][sc].word = source;
        emulator->reg_file[REGISTER][dest].word = destination;
    } else {
        emulator->reg_file[rc][sc].byte[LSB] = source;
        emulator->reg_file[REGISTER][dest].byte[LSB] = destination;
    }
}

/*
 * @brief This function executes the instruction based on the current opcode
 * in the emulator
 */
#ifdef refactor_v1
void execute_0(Emulator *emulator) {
    instruction_data temp_reg;
    unsigned short temp;
    unsigned short old_dest;
    unsigned short destination;
    unsigned short source;
    bool skip_psw_update = false;

    /* local variables copies of operands are made to improve readability */
    unsigned char dest = emulator->inst_operands.dest;
    unsigned char wb = emulator->inst_operands.word_or_byte;
    unsigned char rc = emulator->inst_operands.register_or_constant;
    unsigned char sc = emulator->inst_operands.source_const;

    short index_adjustment;
    //todo refactor this function to not do this anymore, or at least, only doing it for the arithmetic group
    if (wb == WORD) {
        //destination before any operations
        old_dest = emulator->reg_file[REGISTER][dest].word;
        destination = emulator->reg_file[REGISTER][dest].word;
        source = emulator->reg_file[rc][sc].word;
    } else {
        //destination before any operations
        old_dest = emulator->reg_file[REGISTER][dest].byte[LSB];
        destination = emulator->reg_file[REGISTER][dest].byte[LSB];
        source = emulator->reg_file[rc][sc].byte[LSB];
    }

    switch (emulator->opcode) {
        case add:
            destination += source;
            break;
        case addc: // add with carry
            destination += source + emulator->psw.bits.carry;
            break;
        case sub:
            destination += ~source + 1;
            break;
        case subc: // subtract with carry
            destination += ~source + 1 + emulator->psw.bits.carry;
            break;
        case dadd: // decimal add
            bcd_addition(emulator);
            destination = emulator->reg_file[REGISTER][dest].word;
            break;
        case cmp:
            temp = destination + (~source + 1);
            update_psw(temp, emulator, old_dest, ~source);
            break;
        case xor:
            destination ^= source;
            break;
        case and:
            destination &= source;
            break;
        case or:
            destination |= source;
            break;
        case bit: // bit test
            temp = destination & (1 << source);
            update_psw(temp, emulator, old_dest, source);
            break;
        case bic: // bit clear
            destination &= ~(1 << source);
            break;
        case bis: // bit set
            destination |= (1 << source);
            break;
        case mov: // move
            destination = source;
            break;
        case swap:
            temp_reg = emulator->reg_file[REGISTER][sc];
            emulator->reg_file[REGISTER][sc] = emulator->reg_file[REGISTER][dest];
            emulator->reg_file[REGISTER][dest] = temp_reg;
            break;
        case sra: // shift right arithmetic
            ((destination & 0x0001) == 1) ? emulator->psw.bits.carry = 1 : 0;
            if (wb == WORD) {
                (destination & WORD_MSb) == WORD_MSb ? temp = WORD_MSb : 0x0000;
                destination >>= 1;
                destination |= temp;
            } else {
                destination & BYTE_MSb ? temp = BYTE_MSb : 0;
                destination >>= 1;
                destination |= temp;
            }
            break;
        case rrc: // rotate right through carry
            //store current carry
            temp = emulator->psw.bits.carry;
            //update the carry according to our LSB
            emulator->psw.bits.carry = destination & 0x0001;
            //shift destination register
            destination = (destination >> 1);
            //shift carry into MSBit
            wb == WORD ? destination |= temp << WORD_SHIFT : (destination |= temp << BYTE_SHIFT);
            break;
        case swpb: // swap bytes
            temp_reg.byte[MSB] = destination >> 8;
            destination = (destination << 8) | temp_reg.byte[MSB];
            break;
        case sxt: // sign extend
            (destination & BYTE_MSb) == BYTE_MSb ? destination |= 0xFF00 : 0;
            break;
        case ld:
            index_adjustment = calc_index_adjustment(emulator);
            source = emulator->reg_file[REGISTER][sc].word; //ld/st only use register
            rc = REGISTER;
            if((emulator->inst_operands.prpo + emulator->inst_operands.inc + emulator->inst_operands.dec) == 0)
            {
                emulator->d_control.DMAR = source; //EA <-- Source, DMAR <-- EA
            }
            else if (emulator->inst_operands.prpo == 0) //post
            {
                emulator->d_control.DMAR = source;
                source += index_adjustment;
            }
            else //pre
            {
                source += index_adjustment;
                emulator->d_control.DMAR = source;
            }
            emulator->xCTRL = D_READ + wb;
            skip_psw_update = true;
            break;
        case st:
            index_adjustment = calc_index_adjustment(emulator);
            source = emulator->reg_file[REGISTER][sc].word; //ld/st only use register
            rc = REGISTER;
            //CASE UTILIZES FALLTHROUGH INTO STR
            if((emulator->inst_operands.prpo + emulator->inst_operands.inc + emulator->inst_operands.dec) == 0)
            {
                emulator->d_control.DMAR = destination;
            }
            else if (emulator->inst_operands.prpo == 0) //post
            {
                emulator->d_control.DMAR = destination;
                destination += index_adjustment;
            }
            else //pre
            {
                destination += index_adjustment;
                emulator->d_control.DMAR = destination;
            }
            //NOTE FALLTHROUGH OCCURS HERE INTENTIONALLY
        case str:
            //CONTROL CAN ENTER HERE WITH ST
            if(emulator->opcode != st)
            {
                emulator->d_control.DMAR = destination + emulator->offset;
            }
            //THIS EXECUTES WITH AN ST OR STR
            emulator->d_control.DMBR = source;
            emulator->xCTRL = D_WRITE + wb;
            skip_psw_update = true;
            break;
        case ldr:
            emulator->d_control.DMAR = source + emulator->offset;
            emulator->xCTRL = D_READ + wb;
            skip_psw_update = true;
            break;
        case movl:
            emulator->reg_file[REGISTER][dest].byte[LSB] = emulator->move_byte;
            skip_psw_update = true;
            break;
        case movlz:
            emulator->reg_file[REGISTER][dest].byte[LSB] = emulator->move_byte;
            emulator->reg_file[REGISTER][dest].byte[MSB] = 0x00;
            skip_psw_update = true;
            break;
        case movls:
            emulator->reg_file[REGISTER][dest].byte[LSB] = emulator->move_byte;
            emulator->reg_file[REGISTER][dest].byte[MSB] = 0xFF;
            skip_psw_update = true;
            break;
        case movh:
            emulator->reg_file[REGISTER][dest].byte[MSB] = emulator->move_byte;
            skip_psw_update = true;
            break;
/* setcc and clrcc early return to prevent another checking of the PSW. This prevents the PSW from being improperly reset
 * after these instructions have ran*/
        case setcc:
            emulator->psw.word |= emulator->cpu_ops.byte;

            skip_psw_update = true;
            break;
        case clrcc:
            emulator->psw.word &= ~emulator->cpu_ops.byte;
            skip_psw_update = true;
            break;
        case -1:
            break;
        default:
//            printf("Invalid Opcode\n");
            break;
    }
    // Update the destination register with the result
    if(emulator->opcode < movl)
    {
        if (wb == WORD) {
            emulator->reg_file[rc][sc].word = source;
            emulator->reg_file[REGISTER][dest].word = destination;
        } else {
            emulator->reg_file[rc][sc].byte[LSB] = source;
            emulator->reg_file[REGISTER][dest].byte[LSB] = destination;
        }
    }

    // Update the PSW for arithmetic and logical operations
    if (!skip_psw_update) {
        //need to pass in the one's complement of source for subtractions
        if(emulator->opcode == sub || emulator->opcode == subc)
        {
            update_psw(destination, emulator, old_dest, ~source);

        }
        //cmp and bit use the temporary register, and are handled in their individual cases
        else if(emulator->opcode != bit && emulator->opcode != cmp)
        {
            //all other instructions before move take in the same data
            update_psw(destination, emulator, old_dest, source);

        }
    }
#ifdef working
    instruction_data temp_reg;
    unsigned short temp;
    unsigned short old_dest;
    unsigned short destination;
    unsigned short source;

    /* local variables copies of operands are made to improve readability */
    unsigned char dest = emulator->inst_operands.dest;
    unsigned char wb = emulator->inst_operands.word_or_byte;
    unsigned char rc = emulator->inst_operands.register_or_constant;
    unsigned char sc = emulator->inst_operands.source_const;

    if (wb == WORD) {
        old_dest = emulator->reg_file[REGISTER][dest].word;
        destination = emulator->reg_file[REGISTER][dest].word;
        source =  emulator->reg_file[rc][sc].word;
    } else {
        old_dest = emulator->reg_file[REGISTER][dest].byte[LSB];
        destination = emulator->reg_file[REGISTER][dest].byte[LSB];
        source = emulator->reg_file[rc][sc].byte[LSB];
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
                temp = emulator->reg_file[REGISTER][dest].word & (1 << emulator->reg_file[rc][sc].word);
                update_psw(temp, emulator, old_dest, emulator->reg_file[rc][sc].word);
            } else {
                temp = emulator->reg_file[REGISTER][dest].byte[LSB] & (1 << emulator->reg_file[rc][sc].byte[LSB]);
                update_psw(temp, emulator, old_dest, emulator->reg_file[rc][sc].byte[LSB]);
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
#endif
}
#endif
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
    if (emulator->inst_operands.word_or_byte == WORD) {
        ms_bit = WORD_MSb;
        shift_size = WORD_SHIFT;
        emulator->psw.bits.negative = result & ms_bit ? 1 : 0;
    } else {
        ms_bit = BYTE_MSb;
        shift_size = BYTE_SHIFT;
        emulator->psw.bits.negative = result & ms_bit ? 1 : 0;

    }
    emulator->psw.bits.zero = result == 0 ? 1 : 0;
    //only add, sub, addc, subc, and rrc set the carry. RRC handles the carry in its case
    if(emulator->opcode < xor)
    {
        emulator->psw.bits.carry = carry_check[(source & ms_bit) >> shift_size][(old_dest & ms_bit) >> shift_size][(result & ms_bit) >> shift_size];
        emulator->psw.bits.overflow = overflow_check[(source & ms_bit) >> shift_size][(old_dest & ms_bit) >> shift_size][(result & ms_bit) >> shift_size];
    }
    emulator->do_auto_psw_print ?  print_psw(emulator, SINGLE_LINE): printf("") ;
}

#define RESET_HEX 10
/*
 * @brief This function performs a binary coded decimal addition on the source and destination registers
 *
 */
void bcd_addition(Emulator *emulator) {
    //set operand values to local variables
    unsigned char dest = emulator->inst_operands.dest;
    unsigned char wb = emulator->inst_operands.word_or_byte;
    unsigned char rc = emulator->inst_operands.register_or_constant;
    unsigned char sc = emulator->inst_operands.source_const;
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
        wb == WORD ? result_bcd.nibbles.nib2++ : (emulator->psw.bits.carry = 1);
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
            emulator->psw.bits.carry = 1;
        } else {
            emulator->psw.bits.carry = 0;
        }
    }
    if((result_bcd.word == 0 && emulator->inst_operands.word_or_byte == WORD)
    || ((result_bcd.word & 0x00FF) == 0 && emulator->inst_operands.word_or_byte == BYTE))
    {
        emulator->psw.bits.zero = 1;
    }
    else
    {
        emulator->psw.bits.zero = 0;
    }
    //set the result of the bcd addition to the destination register
    emulator->reg_file[REGISTER][dest].word = result_bcd.word;
}