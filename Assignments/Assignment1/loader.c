//
// Created by wyattshaw on 17/05/24.
//
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "loader.h"

#define MAX_RECORD_LEN 70
#define PREAMBLE 8 //8 bytes of preamble for each record
#define ADDRESS_LOCATION 4
#define LENGTH_LOCATION 2
#define TYPE_LOCATION 1
#define CONVERT_FROM_HEX 16
#define CHAR_TO_INT(x) ((x)- '0')



void load(FILE *open_file, char *file_name){
    int record_length;
    int type;
    int record_address;
    if(file_name == NULL)
    {
        printf("No file name to load!\n");
        return;
    }
    open_file = fopen(file_name, "r");
    if(open_file == NULL)
    {
        printf("Error opening file, is the file present?");
        return;
    }
    char s_record[MAX_RECORD_LEN];


    while (fgets(s_record, MAX_RECORD_LEN-1, open_file))
    {
        s_record[strcspn(s_record, "\n")] = 0;
        if(tolower(s_record[0]) != 's')
        {
            printf("Unexpected value in .xme, check file input!\n");
            //return or exit, or go to next entry
        }
        else
        {
            type = CHAR_TO_INT(s_record[TYPE_LOCATION]);
            //2 characters + 1 for NULL, prevents issues with strtol
            char bp_length[3] = {0} ; //byte pair version of length
            strncpy(bp_length, s_record+LENGTH_LOCATION, 2);
            record_length = (int)strtol(bp_length, NULL, CONVERT_FROM_HEX);

            //4 characters + 1 for NULL, prevents issues with strtol
            char bp_address[5] = {0};
            strncpy(bp_address, s_record+ADDRESS_LOCATION, 4);
            record_address = (int)strtol(bp_address, NULL, CONVERT_FROM_HEX);

            //record_length - includes 2 bytes of address data, so we can remove
            //that, it also includes a checksum, but that will be useful to
            //check later
            int string_data_length = (record_length - 2) * 2;
            char* data_string = (char*) malloc(string_data_length * sizeof(char));
            unsigned char* parsed_data = (unsigned char*) malloc(record_length * sizeof (unsigned char));
            strcpy(data_string, s_record + PREAMBLE);
            parse_data(data_string, &parsed_data);
            switch (type)
            {
                case 0:
                    printf("Loaded File: %s\n", parsed_data);
                    break;
                case 1:
                    memcpy(IMEM + record_address, parsed_data, record_length);
                    printf("s1 case\n");
                    break;
                case 2:
                    memcpy(DMEM + record_address, parsed_data, record_length);
                    printf("s2 case\n");
                    break;
                case 9:
                    printf("s9 case\n");
                    break;
                default:
                    printf("Unknown Type");
                    break;
            }
            free(parsed_data);
            free(data_string);

        }
    }

}

void menu(){
    char command = '\0';
    do
    {
        printf("Enter a command, Load (L), Display Memory (M), "
               "Q to quit: ");
        scanf("%c", &command);
        if(tolower(command) == 'l')
        {
            char input_string[MAX_RECORD_LEN];
            printf("Enter name of file to load:");
            scanf("%s", input_string);
            load(input_file, input_string);
        }
        else if(tolower(command) == 'm')
        {
            display_data();
        }
        getchar();
    }while(tolower(command) != 'q');
}

void display_data() {
    char upper[4] = {0};
    char lower[4] = {0};
    char mem_type;
    printf("Enter lower, upper, memory type (I/D)");
    scanf("%s %s %c", lower, upper, &mem_type);
    printf("Entered Memory bounds %s --> %s, searching mem_block %c\n", lower, upper, mem_type);

    int lower_lookup = (int)strtol(lower, NULL, CONVERT_FROM_HEX);
    int upper_lookup = (int)strtol(upper, NULL, CONVERT_FROM_HEX);

    if(tolower(mem_type) == 'i')
    {
        for (int i = lower_lookup; i < upper_lookup ; ++i)
        {
            printf("%02x, ", IMEM[i]);
            if(i > lower_lookup && i+1 % 16 == 0)
            {
                printf("\n");
            }
        }
    }
    else if(tolower(mem_type) == 'd')
    {
        for (int i = lower_lookup; i < upper_lookup ; ++i)
        {
            printf("%02x, ", DMEM[i]);
            if(i > lower_lookup && (i+1) % 16 == 0)
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

void parse_data(char *string_data, unsigned char **data_to_change) {
    if(string_data == NULL)
    {
        printf("string passed to funciton to parse!\n");
        return;
    }
    else if(strlen(string_data) % 2 != 0)
    {
        printf("Missing data, bytepairs will always give even string!\n");
        return;
    }
    else
    {
        char str_to_convert[2];
        int data_index = 0;
        for (int i = 0; i < strlen(string_data) -2; i+=2)
        {
            strncpy(str_to_convert, string_data + i, 2);
            (*data_to_change)[data_index] = strtol(str_to_convert, NULL, 16);
            data_index++;
        }
    }

}