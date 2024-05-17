//
// Created by wyattshaw on 17/05/24.
//

#ifndef ASSIGNMENT1_LOADER_H
#define ASSIGNMENT1_LOADER_H


void load(FILE *open_file, char *file_name);
void menu();
void display_data();
void parse_data(char *string_data, unsigned char **data_to_change);
int test_checksum(char* data, unsigned char hex);

#define xMEM_SIZE 1<<16

extern FILE* input_file;
extern unsigned char IMEM[xMEM_SIZE];
extern unsigned char DMEM[xMEM_SIZE];
#endif //ASSIGNMENT1_LOADER_H
