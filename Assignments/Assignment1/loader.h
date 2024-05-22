//
// Created by wyattshaw on 17/05/24.
//

#ifndef ASSIGNMENT1_LOADER_H
#define ASSIGNMENT1_LOADER_H

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


#define SIZE_OF_RAW_LENGTH 3 //2 chars + NULL
#define SIZE_OF_RAW_ADDRESS 5 //4 chars + NULL
#define SIZE_OF_RAW_CHECKSUM 3 //2 chars + NULL

//#define TWO_MEM_ARRAY

typedef union memory
{
    unsigned char byte[BYTE_MEMORY_SIZE];
    unsigned short word[WORD_MEMORY_SIZE];
}Memory;


void load(FILE *open_file, char *file_name);
void menu();
void display_data();
void parse_data(char *string_data, unsigned char **converted_data);
bool test_checksum(unsigned char *data, unsigned char length_byte, char *address_bytes, char *checksum_byte, int data_length);
void clean_data(char* s_record);
void store_in_memory(int type, int record_address, int record_length, unsigned char* parsed_data);
bool record_check(char * s_record);
void display_loader_memory();



extern FILE* input_file;
extern unsigned char IMEM[BYTE_MEMORY_SIZE];
extern unsigned char DMEM[BYTE_MEMORY_SIZE];
extern Memory loader_memory[2];
#endif //ASSIGNMENT1_LOADER_H
