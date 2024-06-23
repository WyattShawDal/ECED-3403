/*
 * File Name: emulation.c
 * Date June 20 2024
 * Written By: Wyatt Shaw
 * Module Info: This module defines the functions for the operation of the emulator
 *
 * !!NOTE!! Breakpoint will break *after* the instruction. This is atypical for breakpoints, and may be modified in
 * the future, but for now it is suitable as it allows the developer to view the post effect of the instruction quickly
 * IE PSW.
 */
#include "emulation.h"
#include <signal.h>
#include <unistd.h>

/*
 * @brief interrupt handler for halting the emulation using ctrl-c without exiting the program
 */
volatile sig_atomic_t stop_loop;
void int_handler(int signum)
{
    stop_loop = 1;
}

/*
 * @brief This function implements the simulation of the emnulator, it provides an
 * update to clock cycles and handles the calling of functions according to pipeline
 * stages
 *
 */
#define IS_EVEN(x) (x % 2 == 0)
#define EVEN 1
#define ODD 0
void run_emulator(Emulator *emulator)
{
    /*allows us to handle SIGINT (CTRL-C gracefully and stop the while loop) without exiting the process*/
    signal(SIGINT, int_handler);
    if(emulator->has_started)
    {
        printf("Emulator is already running\n");
        return;
    }
    emulator->has_started = true;
    do
    {

        //this if else, combo implements the pipeline
        if(IS_EVEN(emulator->clock))
        {
            emulator->is_single_step ? printf("Start PC: %04x, BRKPT: %04x, CLK: %d\n", emulator->reg_file[REGISTER][PROG_COUNTER].word, emulator->breakpoint, emulator->clock) : printf("");
            fetch_instruction(emulator, EVEN); //f0
            decode_instruction(emulator); //d
        }

        else
        {
            fetch_instruction(emulator, ODD); //f1
            execute_instruction(emulator); //e
        }
        //after pipeline stages increment clock
        emulator->clock++;
        //now we should check if we should open the menu
        if ((emulator->is_single_step == true && IS_EVEN(emulator->clock)) || emulator->reg_file[REGISTER][PROG_COUNTER].word == emulator->breakpoint)
        {
            printf("END PC: %04x, BRKPT: %04x, CLK: %d\n", emulator->reg_file[REGISTER][PROG_COUNTER].word, emulator->breakpoint, emulator->clock);
            menu(emulator);
        }
        //then check if an interrupt has occurred, which will also open the menu. This could be place in the above loop with another or
        //however, since the reason for pausing is different, and this section involves the interrupt, separation was desired for clarity.
        else if(stop_loop)
        {
            //resetting the signal handler
            signal(SIGINT, int_handler);
            stop_loop = 0;
            //pausing the emulator
            printf("Halting Emulator\n");
            emulator->is_user_interrupt = true;
            menu(emulator);
        }
    } while (emulator->has_started);
    printf("EMULATION ENDED WITH PC: %d\nCLOCK: %d", emulator->reg_file[REGISTER][PROG_COUNTER].word, emulator->clock);
}

/*
 * @brief This function implements the fetch portion of the xm23p processor, setting the IMAR, updating the PC, and setting the xCTRL
 * to ICRTL. On the odd clock cycles, the function will set the instruction register to the value of the memory buffer register
 */
void fetch_instruction(Emulator *emulator, int even)
{
    if(even)//f0
    {
        //set the IMAR to the current program counter
        emulator->i_control.IMAR = emulator->reg_file[REGISTER][PROG_COUNTER].word;
        //get a new program counter
        emulator->reg_file[REGISTER][PROG_COUNTER].word += 2;
        //set xCTRL for the memory controller
        emulator->xCTRL = I_MEMORY;
    }
    else//f1
    {
        //call the memory_controller to load a new instruction into the memory buffer register
        memory_controller(emulator);
        //set the instruction register to the IMBR
        //instruction register is used during decoding
        emulator->instruction_register = emulator->i_control.IMBR;
    }
}

/*
 * @brief This function sets the memory buffer register to the value of the memory at the memory address register
 * @note technically not needed, but better emulates the xm23p
 */
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
/*
 * @brief set some inital values in the emulator
 */
void init_emulator(Emulator *emulator)
{
    emulator->breakpoint = (BYTE_MEMORY_SIZE) - 1;;
    instruction_data reg_file[REG_FILE_OPTIONS][REGFILE_SIZE] = {
            {
                    { .word = 0 }, { .word = 0 }, { .word = 0 }, { .word = 0 },
                    { .word = 0 }, { .word = 0 }, { .word = 0 }, { .word = 0 }
            },
            {
                    { .word = 0 }, { .word = 1 }, { .word = 2 }, { .word = 4 },
                    { .word = 8 }, { .word = 16 }, { .word = 32 }, { .word = (unsigned short)-1 }
            }
    };
    memcpy(emulator->reg_file, reg_file, sizeof (unsigned short) * REG_FILE_OPTIONS * REGFILE_SIZE);
}

/*
 * @brief menu provides a menu for the user to interact with the xm-23p emulator
 */
void menu(Emulator *emulator) {
    char command = '\0';
    char input_string[MAX_RECORD_LEN];
    //if the emulator hasn't yet started print out all the options, this is to prevent printing everytime we call menu
    if(!emulator->has_started)
    {
        printf("Commands:\n"
               "Begin Emulation (G)\nEnable Single Step (S)\nLoad (L)\nDisplay Memory (M)\n");
        printf("Print PSW (P)\nPrint Registers (R)\nModify Register Values (T)\n"
               "Modify Memory Value (U)\nReset PSW (Z)\nSet Breakpoint Value (Y)\nQuit (Q)\n");

    }
    do
    //enter loop for entering commands, this allows us to do multiple commands from one menu() call
    {
        printf("Enter Option (? for list of commands): ");
        if (scanf(" %c", &command) != 1)
        {
            //clearing the input buffer
            while (getchar() != '\n');
            continue;
        }
        command = (char) tolower(command);
        switch(command)
        {
            case 'l':
                printf("Enter name of file to load:");
                scanf("%70s", input_string);
                load(input_file, input_string, emulator);
                emulator->is_memset = true;
                break;
            case 'm':
                display_loader_memory();
                break;
            case 'g':
                //no file loaded
                if(!emulator->is_memset)
                {
                    printf("No file loaded, cannot run emulator\n");
                    break;
                }
                //if it's single_step and the emulator has started we just want to move to the next clock cycle
                else if(emulator->has_started && emulator->is_single_step)
                {
                    return;
                }
                //if the loop was interrupted and is now resuming print a message reflecting that
                //reset user_interrupt variable
                else if (emulator->has_started && emulator->is_user_interrupt)
                {
                    printf("Resuming emulation\n");
                    emulator->is_user_interrupt = false;
                    return;
                }
                //if the emulator has started and isn't in single step mode, we don't want to start it again
                else if(emulator->has_started)
                {
                    printf("Emulator is already running\n");
                    return;
                }
                //if the emulator hasn't started, we can start it
                else
                {
                    printf("Running Emulator\n");
                    run_emulator(emulator);
                    break;
                }
            case 's':
                //toggle single step
                emulator->is_single_step ? printf("[disabling]") : printf("[enabling]");
                printf(" single step\n");
                emulator->is_single_step = !emulator->is_single_step;
                break;
            case 'p':
                print_psw(emulator);
                break;
            case 'r':
                print_registers(emulator);
                break;
            case 't':
                modify_registers(emulator);
                break;
            case 'u':
                modify_memory_locations(emulator);
                break;
            case 'y':
                set_breakpoint(emulator);
                break;
            case 'q':
                printf("Exiting menu\n");
                break;
            case '?':
                printf("Commands:\n"
                       "Begin Emulation (G)\nEnable Single Step (S)\nLoad (L)\nDisplay Memory (M)\nDebug Menu (D)\nQuit (Q)\n");
                break;
            case 'z':
                printf("Clearing PSW\n");
                memset(&emulator->psw, 0, sizeof(program_status_word));
                break;
            default:
                printf("Invalid command, try again\n");
                break;
        }
        while (getchar() != '\n'); //another buffer clear to ensure menu doesn't loop
    }
    while (tolower(command) != 'q');
    if(emulator->has_started)
    {
        printf("Exiting Emulator...");
        getchar();
        exit(0);
    }
}