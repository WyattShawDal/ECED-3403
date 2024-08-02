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
#define IS_EVEN(x) (x % 2 == 0)
#define EVEN 1
#define ODD 0
/*
 * @brief This function implements the simulation of the emulator, it provides an
 * update to clock cycles and handles the calling of functions according to pipeline
 * stages
 */

void run_emulator(Emulator *emulator)
{
    //allows us to handle SIGINT (CTRL-C gracefully and stop the while loop) without exiting the process
    unsigned short previously_decoded;
    signal(SIGINT, int_handler);
    //guard to prevent repeat starting of the emulator
    if(emulator->has_started)
    {
        printf("Emulator is already running\n");
        return;
    }
    //set the emulator to started
    emulator->has_started = true;
    //print the header for the pipeline
    printf("CLK    PC    INST    FETCH     DECODE    EXECUTE        PSW         CEX\n");
    do
    {
        //if the clock is even, we do the even cycle
        if(IS_EVEN(emulator->clock))
        {
            previously_decoded = do_even_cycle(emulator);
        }
        //if the clock is odd, we do the odd cycle
        else
        {
            do_odd_cycle(emulator, previously_decoded);
        }
        //after pipeline stages increment clock
        emulator->clock++;
        //pause every clocktick
        if(emulator->stop_on_clock && emulator->is_single_step == true)
        {
            menu(emulator);
        }
        //pause every pc increment
        else if ((emulator->is_single_step == true && IS_EVEN(emulator->clock)) && emulator->stop_on_clock == false)
        {
            menu(emulator);
        }
        //pause on user interrupt
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



unsigned short do_even_cycle(Emulator *emulator)
{
    printf("%-5lu %04X   %04X   ", emulator->clock, emulator->reg_file[REGISTER][PROG_COUNTER].word, xm23_memory[I_MEMORY].word[emulator->reg_file[REGISTER][PROG_COUNTER].word >> 1]);

    //executes for load stores D-MEM reads and D-MEM writes
    if(emulator->xCTRL != I_MEMORY && emulator->xCTRL != NO_ACCESS)
    {
        execute_1(emulator); //e1
        emulator->xCTRL = NO_ACCESS;
    }
    char cond_exec_char = emulator->cond_exec.status == CEX_TRUE ? 'T' : (emulator->cond_exec.status == CEX_FALSE ? 'F' : 'X');
    printf("F0: %04X  ", emulator->reg_file[REGISTER][PROG_COUNTER].word);

    fetch_instruction(emulator, EVEN); //f0
    if(emulator->cond_exec.status == CEX_TRUE)
    {
        //run as normal for TRUE count
        if(emulator->cond_exec.true_count)
        {
            decode_instruction(emulator); //d0
            printf("D0: %04X                           CEX:%c%d%d\n", emulator->instruction_register,cond_exec_char, emulator->cond_exec.true_count, emulator->cond_exec.false_count);
            emulator->cond_exec.true_count--;
        }
        //
        else if(emulator->cond_exec.false_count)
        {
            printf("D0: BUB.                           CEX:%c%d%d\n", cond_exec_char, emulator->cond_exec.true_count, emulator->cond_exec.false_count);
            emulator->hazard_control.d_bubble = true;
            emulator->hazard_control.e_bubble = true;
            emulator->cond_exec.false_count--;
        }
        else
        {
            decode_instruction(emulator); //d0
            printf("D0: %04X                           CEX:%c%d%d\n", emulator->instruction_register,cond_exec_char, emulator->cond_exec.true_count, emulator->cond_exec.false_count);
            emulator->hazard_control.d_bubble = false;
            emulator->hazard_control.e_bubble = false;
            emulator->cond_exec.status = CEX_DISABLED;

        }
    }
    else if(emulator->cond_exec.status == CEX_FALSE)
    {
        //skip for TRUE count
        if(emulator->cond_exec.true_count)
        {
            printf("D0: BUB.                           CEX:%c%d%d\n", cond_exec_char, emulator->cond_exec.true_count, emulator->cond_exec.false_count);
            emulator->hazard_control.d_bubble = true;
            emulator->hazard_control.e_bubble = true;
            emulator->cond_exec.true_count--;
        }
        //run as normal for FALSE count
        else if(emulator->cond_exec.false_count)
        {
            decode_instruction(emulator); //d0
            printf("D0: %04X                           CEX:%c%d%d\n", emulator->instruction_register,cond_exec_char, emulator->cond_exec.true_count, emulator->cond_exec.false_count);
            emulator->cond_exec.false_count--;
        }
        else
        {
            decode_instruction(emulator); //d0
            printf("D0: %04X                           CEX:%c%d%d\n", emulator->instruction_register,cond_exec_char, emulator->cond_exec.true_count, emulator->cond_exec.false_count);
            emulator->hazard_control.d_bubble = false;
            emulator->hazard_control.e_bubble = false;
            emulator->cond_exec.status = CEX_DISABLED;

        }
    }
    else if(emulator->hazard_control.d_bubble)
    {
        printf("D0: BUB.                           CEX:%c%d%d\n", cond_exec_char, emulator->cond_exec.true_count, emulator->cond_exec.false_count);
        emulator->hazard_control.d_bubble = false;
    }
    else
    {
        decode_instruction(emulator); //d0
        printf("D0: %04X                           CEX:%c%d%d\n", emulator->instruction_register,cond_exec_char, emulator->cond_exec.true_count, emulator->cond_exec.false_count);
    }
    return emulator->instruction_register;
}

void do_odd_cycle(Emulator *emulator, unsigned short previously_decoded)
{
    fetch_instruction(emulator, ODD); //f1

    printf("%-5lu               F1: %04X             ",emulator->clock, emulator->i_control.IMBR);
    char cond_exec_char = emulator->cond_exec.status == CEX_TRUE ? 'T' : (emulator->cond_exec.status == CEX_FALSE ? 'F' : 'X');
    if(emulator->hazard_control.e_bubble)
    {
        emulator->hazard_control.e_bubble = false;
        printf("E0: BUB.    VNZC: %1d%1d%1d%1d  CEX:%c%d%d\n", emulator->psw.bits.overflow, emulator->psw.bits.negative, emulator->psw.bits.zero, emulator->psw.bits.carry,
               cond_exec_char, emulator->cond_exec.true_count, emulator->cond_exec.false_count);
    }
    else
    {
        execute_0(emulator); //e0
        printf("E0: %04X    VNZC: %1d%1d%1d%1d  CEX:%c%d%d\n", previously_decoded, emulator->psw.bits.overflow, emulator->psw.bits.negative, emulator->psw.bits.zero, emulator->psw.bits.carry,
               cond_exec_char, emulator->cond_exec.true_count, emulator->cond_exec.false_count);

    }

    //break after instruction has been executed
    if(emulator->reg_file[REGISTER][PROG_COUNTER].word == emulator->breakpoint)
    {
        emulator->has_started =false;
        emulator->hide_menu_prompt = false;
        menu(emulator);
    }
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
 *
 */
void memory_controller(Emulator *emulator)
{
    if(emulator->xCTRL == I_MEMORY )
    {
        emulator->i_control.IMBR = xm23_memory[I_MEMORY].word[emulator->i_control.IMAR >> 1];
    }
    else
    {
        switch (emulator->xCTRL)
        {
            case D_READ:
                emulator->d_control.DMBR = xm23_memory[D_MEMORY].word[emulator->d_control.DMAR >> 1];
                break;
            case D_READ_B:
                emulator->d_control.DMBR = xm23_memory[D_MEMORY].byte[emulator->d_control.DMAR];
                break;
            case D_WRITE:
                xm23_memory[D_MEMORY].word[emulator->d_control.DMAR >> 1] = emulator->d_control.DMBR;
                break;
            case D_WRITE_B:
                xm23_memory[D_MEMORY].byte[emulator->d_control.DMAR] = emulator->d_control.DMBR;
                break;
            case NO_ACCESS:
            case D_MEMORY:
                return;
        }
    }

}
/*
 * @brief set some inital values in the emulator
 */
void init_emulator(Emulator *emulator)
{
    //emulator is created via a calloc so we can just initialize values that are not going to be zero here
    emulator->breakpoint = (BYTE_MEMORY_SIZE) - 1;
    emulator->stop_on_clock = true;
    emulator->xCTRL = NO_ACCESS;
    emulator->hazard_control.d_bubble = true;
    emulator->hazard_control.e_bubble = true;
    emulator->cond_exec.status = CEX_DISABLED;
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
void print_menu_options()
{
    printf("Commands:\n"
           "Begin Emulation (G)\nEnable Single Step (S)\nLoad (L)\nDisplay Memory (M)\nPrint PSW (P)\nPrint Registers (R)"
           "\nModify Register Values (T)\nModify Memory Value (U)\nReset PSW (Z)\nSet Breakpoint Value (Y)\nHide Menu Prompt (H)\n"
           "Stop Execution on Clock (X)\nQuit (Q)\n");
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
        print_menu_options();
    }
    do
        //enter loop for entering commands, this allows us to do multiple commands from one menu() call
    {
        if(!emulator->hide_menu_prompt) printf("Enter Option (? for list of commands): ");
        if (scanf(" %c", &command) != 1)
        {
            //clearing the input buffer
            while (getchar() != '\n');
            continue;
        }
        command = (char) tolower(command);
        switch(command)
        {
            case 'x':
                emulator->stop_on_clock ? printf("[disabling]") : printf("[enabling]");
                printf(" stop on clock\n");
                emulator->stop_on_clock = !emulator->stop_on_clock;
                break;
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
                print_psw(emulator, MULTI_LINE);
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
            case 'h':
                emulator->hide_menu_prompt ? printf("[disabling]") : printf("[enabling]");
                printf(" hide menu prompt\n");
                emulator->hide_menu_prompt = !emulator->hide_menu_prompt;
                break;
            case '?':
                print_menu_options();
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