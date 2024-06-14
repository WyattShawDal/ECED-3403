//
// Created by wyattshaw on 06/06/24.
//
#include "emulation.h"

/*
 * @brief This function implements the simulation of the emnulator, it provides an
 * update to clock cycles and handles the calling of functions according to pipeline
 * stages
 * TODO Implement the fetch_instruction function
 * TODO Modify all functions that change the emulator to take in a pointer to the emulator
 */

void run_emulator(Emulator *emulator)
{
    if(emulator->is_emulator_running)
    {
        printf("Emulator is already running\n");
        return;
    }
    do
    {
        if(emulator->clock % 2 == 0)
        {
            //fetch_instruction(emulator, EVEN);
            decode_instruction(NULL);
        }
        else
        {
            //fetch_instruction(emulator, EVEN);
            execute_instruction(emulator);
        }
        emulator->clock++;
        if (emulator->is_single_step == true)
        {
            emulator->is_emulator_running = false;
            menu();
        }
    } while (emulator->is_emulator_running);
    printf("ENDED WITH PC: %d\nCLOCK: %d", emulator->reg_file[REGISTER][PROG_COUNTER].word, emulator->clock);
}