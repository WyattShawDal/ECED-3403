//
// Created by wyattshaw on 06/06/24.
//
#include "emulation.h"

/*
 * @brief This function implements the simulation of the emnulator, it provides an
 * update to clock cycles and handles the calling of functions according to pipeline
 * stages
 * TODO Modify all functions that change the emulator to take in a pointer to the emulator
 */

#define EVEN 1
#define ODD 0
void run_emulator(Emulator *emulator)
{
    if(emulator->is_emulator_running)
    {
        printf("Emulator is already running\n");
        return;
    }
    emulator->is_emulator_running = true;
    do
    {
        if(emulator->clock % 2 == 0)
        {
            fetch_instruction(emulator, EVEN);
            decode_instruction(emulator);
        }
        else
        {
            fetch_instruction(emulator, ODD);
            execute_instruction(emulator);
        }
        emulator->clock++;
        if (emulator->is_single_step == true || emulator->reg_file[REGISTER][PROG_COUNTER].word + 2 >= emulator->breakpoint)
        {
            menu();
        }
    } while (emulator->is_emulator_running);
    printf("ENDED WITH PC: %d\nCLOCK: %d", emulator->reg_file[REGISTER][PROG_COUNTER].word, emulator->clock);
}

void fetch_instruction(Emulator *emulator, int even)
{
    if(even)
    {
        emulator->i_control.IMAR = emulator->reg_file[REGISTER][PROG_COUNTER].word;
        emulator->reg_file[REGISTER][PROG_COUNTER].word += 2;
        emulator->xCTRL = I_MEMORY;
    }
    else
    {
        memory_controller(emulator);
        emulator->instruction_register = emulator->i_control.IMBR;
    }
}

void memory_controller(Emulator *emulator)
{
    if(emulator->xCTRL == I_MEMORY )
    {
        emulator->i_control.IMBR = xm23_memory[I_MEMORY].word[emulator->i_control.IMAR >> 1];
    }
    else
    {
        emulator->d_control.DMBR = xm23_memory[D_MEMORY].word[emulator->d_control.DMAR >> 1];
    }

}