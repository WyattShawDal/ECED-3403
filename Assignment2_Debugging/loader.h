//
// Created by wyattshaw on 17/05/24.
//

#ifndef ASSIGNMENT1_LOADER_H
#define ASSIGNMENT1_LOADER_H
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


#define INSTR 0
#define DATA 1
#define TYPE_LOCATION 1
#define LENGTH_LOCATION 2
#define CHECKSUM_LENGTH 2
#define BYTE_SIZE 2 //character representation of a byte in hex (xx)
#define ADDRESS_LOCATION 4
#define PREAMBLE 8 //8 bytes of preamble for each record
#define CONVERT_FROM_HEX 16
#define MEMORY_LINE_LENGTH 16
#define MAX_RECORD_LEN (70+1)
#define VALID_CHECKSUM 255
#define BYTE_MEMORY_SIZE 1<<16
#define WORD_MEMORY_SIZE 1<<15
#define REG_FILE_OPTIONS 2
#define REGFILE_SIZE 8
#define REGISTER 0
#define CONSTANT 1
#define SIZE_OF_RAW_LENGTH 3 //2 chars + NULL
#define SIZE_OF_RAW_ADDRESS 5 //4 chars + NULL
#define SIZE_OF_RAW_CHECKSUM 3 //2 chars + NULL

//#define TWO_MEM_ARRAY
typedef enum
{
    I_MEMORY = 0,
    D_MEMORY = 1,
    D_READ = 2,
    D_READ_B = 3,
    D_WRITE = 4,
    D_WRITE_B = 5,
    NO_ACCESS = 6
}MEMORY_ACCESS_TYPES;

typedef union memory
{
    unsigned char byte[BYTE_MEMORY_SIZE];
    unsigned short word[WORD_MEMORY_SIZE];
}Memory;
//forward declaration
typedef struct emulator_data Emulator;

void load(FILE *open_file, char *file_name, Emulator *emulator);
void parse_data(char *string_data, unsigned char **converted_data,
                unsigned char *sum);

bool test_checksum(unsigned char length_byte, unsigned short address_bytes,
                   char *checksum_byte, unsigned char data_sum);
void clean_data(char *s_record, Emulator *emulator);
void store_in_memory(int type, int record_address, int record_length, unsigned char *parsed_data, Emulator *emulator);
bool record_check(char * s_record);
void display_loader_memory();



extern FILE* input_file;
extern unsigned char IMEM[BYTE_MEMORY_SIZE];
extern unsigned char DMEM[BYTE_MEMORY_SIZE];
extern Memory xm23_memory[2];
#endif //ASSIGNMENT1_LOADER_H
