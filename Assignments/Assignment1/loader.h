//
// Created by wyattshaw on 17/05/24.
//

#ifndef ASSIGNMENT1_LOADER_H
#define ASSIGNMENT1_LOADER_H

#define TYPE_LOCATION 1
#define LENGTH_LOCATION 2
#define CHECKSUM_LENGTH 2
#define ADDRESS_LOCATION 4
#define PREAMBLE 8 //8 bytes of preamble for each record
#define CONVERT_FROM_HEX 16
#define MEMORY_LINE_LENGTH 16
#define MAX_RECORD_LEN (70+1)
#define xMEM_SIZE 1<<16

#define SIZE_OF_RAW_LENGTH 3 //2 chars + NULL
#define SIZE_OF_RAW_ADDRESS 5 //4 chars + NULL
#define SIZE_OF_RAW_CHECKSUM 3 //2 chars + NULL

#define SIZE_OF_USER_ADDRESS 5


void load(FILE *open_file, char *file_name);
void menu();
void display_data();
void parse_data(char *string_data, unsigned char **converted_data, int data_length);
bool test_checksum(unsigned char *data, char *length_byte, char *address_bytes,char *checksum_byte, int data_length);
void clean_data(char* s_record);
void store_in_memory(int type, int record_address, int record_length, unsigned char* parsed_data);
bool record_check(char * s_record);




extern FILE* input_file;
extern unsigned char IMEM[xMEM_SIZE];
extern unsigned char DMEM[xMEM_SIZE];
#endif //ASSIGNMENT1_LOADER_H
