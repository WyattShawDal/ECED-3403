/*
 * File Name: loader.c
 * Date May 18 2024
 * Written By: Wyatt Shaw
 * Module Info: The module implements the loaders functionality,  including
 * A menu option, file loading, data parsing/storage, and display for verifying
 * laoding success.
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "loader.h"

#define CHAR_TO_INT(x) ((x)- '0')

void menu() {
    char command = '\0';
    do
    {
        printf("Enter a command, Load (L), Display Memory (M), Quit (Q): ");
        scanf(" %c", &command);
        if (tolower(command) == 'l')
        {
            char input_string[MAX_RECORD_LEN];
            printf("Enter name of file to load:");
            scanf("%s", input_string);
            load(input_file, input_string);
        }
        else if (tolower(command) == 'm')
        {
            display_data();
        }
        getchar();
    }
    while (tolower(command) != 'q');
    printf("Exiting Loader...");
}

void load(FILE *open_file, char *file_name) {
    if (file_name == NULL)
    {
        printf("No file name to load!\n");
        return;
    }
    open_file = fopen(file_name, "r");
    if (open_file == NULL)
    {
        printf("Error opening file, is the file present?\n");
        return;
    }
    char s_record[MAX_RECORD_LEN];

    while (fgets(s_record, MAX_RECORD_LEN, open_file))
    {
        s_record[strcspn(s_record, "\n")] = 0;
        if (tolower(s_record[0]) != 's')
        {
            printf("Unexpected value in .xme, check file input!\n");
            //go to next line
        }
        else
        {
            if(record_check(s_record))
            {
                clean_data(s_record);
            }
            else
            {
                return;
            }
        }
    }
}

bool record_check(char * s_record){
    for (int i = 1; i < strlen(s_record); ++i)
    {
        if(!isalnum(s_record[i]))
        {
            printf("Unexpected value {%d} in s_record, possibly corrupt value.. Aborting load\n", s_record[i]);
            return false;
        }
        if(toupper(s_record[i]) > 'F' && (s_record[i] != '\n' && s_record[i] !='\0') )
        {
            printf("Unexpected hex character {%c} in s_record possibly corrupt value.. Aborting load\n", s_record[i]);
            return false;
        }
    }
    return true;
}

void clean_data(char *s_record) {
    int processed_length;
    int type;
    int processed_address;
    type = CHAR_TO_INT(s_record[TYPE_LOCATION]);

    char raw_length[SIZE_OF_RAW_LENGTH] = {0}; //byte pair version of length
    strncpy(raw_length, s_record + LENGTH_LOCATION, 2);
    processed_length = (int) strtol(raw_length, NULL, CONVERT_FROM_HEX);

    char raw_address[SIZE_OF_RAW_ADDRESS] = {0};
    strncpy(raw_address, s_record + ADDRESS_LOCATION, 4);
    processed_address = (int) strtol(raw_address, NULL, CONVERT_FROM_HEX);

    char raw_checksum[SIZE_OF_RAW_CHECKSUM] = {0};
    //copy checksum from end of s_record into it's own array
    strncpy(raw_checksum, s_record + (strlen(s_record) - 2), 2);
    //create an array of bytes to hold the integer version of our data
    unsigned char *parsed_data = (unsigned char *) calloc(processed_length,
                                                          sizeof(unsigned char));
    //parse the data, array passed by reference because copying is wasteful in this case
    parse_data(s_record + PREAMBLE, &parsed_data, processed_length);
    //verify our data values pair with the checksum provided by the assembler
    if (test_checksum(parsed_data, raw_length, raw_address, raw_checksum,
                      processed_length))
    {
        store_in_memory(type, processed_address, processed_length, parsed_data);
    }
    else
    {
        printf("Failed to verify checksum, possibly corrupt file, aborting load.\n");
    }
    memset(s_record, '\0', MAX_RECORD_LEN);
    //free(parsed_data);
    //free(data_string);

}

void store_in_memory(int type, int record_address, int record_length,
                     unsigned char *parsed_data) {
    switch (type)
    {
        case 0:
            printf("Loaded File: %s\n", parsed_data);
            break;
        case 1:
            memcpy(IMEM + record_address, parsed_data, record_length);
            printf("S1 Stored\n");
            break;
        case 2:
            memcpy(DMEM + record_address, parsed_data, record_length);
            printf("S2 Stored\n");
            break;
        case 9:
            //memcpy(IMEM + record_address, parsed_data, record_length);
            printf("File read successfully. Starting Addr = %04x\n",
                   record_address);
            break;
        default:
            printf("Unknown Type");
            break;
    }
}

void display_data() {
    char upper[SIZE_OF_USER_ADDRESS] = {0};
    char lower[SIZE_OF_USER_ADDRESS] = {0};
    char mem_type;
    printf("Enter lower, upper, memory type (I/D)");
    scanf("%4s %4s %c", lower, upper, &mem_type);
    printf("Entered Memory bounds %s --> %s,", lower, upper);

    int lower_lookup = (int) strtol(lower, NULL, CONVERT_FROM_HEX);
    int upper_lookup = (int) strtol(upper, NULL, CONVERT_FROM_HEX);

    if (tolower(mem_type) == 'i')
    {
        printf("Searching Instruction memory: \n");
        for (int i = lower_lookup; i < upper_lookup; ++i)
        {
            if (i % MEMORY_LINE_LENGTH == 0)
            {
                printf("%04x: ", i);
            }
            printf("%02x, ", IMEM[i]);
            if (i > lower_lookup && (i + 1) % MEMORY_LINE_LENGTH == 0)
            {
                printf("\n");
            }
        }
    }
    else if (tolower(mem_type) == 'd')
    {
        printf("Searching Data memory: \n");
        for (int i = lower_lookup; i < upper_lookup; ++i)
        {
            if (i % MEMORY_LINE_LENGTH == 0) //print starting address
            {
                printf("%04x: ", i);
            }
            printf("%02x, ", DMEM[i]);
            if (i > lower_lookup &&
                (i + 1) % MEMORY_LINE_LENGTH == 0) //newline after 16 bytes printed
            {
                printf("\n");
            }
        }
    }
    else
    {
        printf("Unknown value entered\n");
    }
}

void parse_data(char *string_data, unsigned char **converted_data, int data_length) {
    if (string_data == NULL) //check if the entire set of data is missing
    {
        printf("string passed to funciton to parse!\n");
        return;
    }
    else if (strlen(string_data) % 2 != 0) //check if there is an extra or missing char
    {
        printf("\n");
        return;
    }
    else
    {
        char str_to_convert[2]; //buffer for converting pair of chars into one int value
        int data_index = 0; //track where we are in the converted data
        for (int i = 0; i < strlen(string_data) - CHECKSUM_LENGTH; i += 2)
        {
            //copy 2 chars at a time into the conversion buffer
            strncpy(str_to_convert, string_data + i, 2);
            //store the into version in the converted buffer
            (*converted_data)[data_index] = strtol(str_to_convert, NULL,
                                                   CONVERT_FROM_HEX);
            data_index++;
        }
    }

}

bool test_checksum(unsigned char *data, char *length_byte, char *address_bytes,
                   char *checksum_byte, int data_length) {

    bool rc = false;
    unsigned char sum = 0;
    //convert length byte to unsigned char using strtol
    unsigned char lb = strtol(length_byte, NULL, CONVERT_FROM_HEX);

    unsigned char cb = strtol(checksum_byte, NULL, CONVERT_FROM_HEX);
    //we need to split the address into individual bytes instead of converting the two
    char split[2];
    memcpy(split, address_bytes, 2);
    unsigned char ab1 = strtol(split, NULL, CONVERT_FROM_HEX);
    memcpy(split, address_bytes + 2, 2);
    unsigned char ab2 = strtol(split, NULL, CONVERT_FROM_HEX);


    sum = lb + ab1 + ab2;
    //sum up to the checksum
    for (int i = 0; i < data_length - CHECKSUM_LENGTH; ++i)
    {
        sum += data[i];
    }
    printf("Check Value is %d\n", sum + cb);
    if (sum + cb == 255)
    {
        rc = true;
    }
    return rc;
}