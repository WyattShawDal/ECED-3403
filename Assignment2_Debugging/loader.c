/*
 * File Name: loader.c
 * Date May 18 2024
 * Written By: Wyatt Shaw
 * Module Info: The module implements the loaders functionality,  including
 * A menu option, file loading, data parsing/storage, and display for verifying
 * laoding success.
 */

#include "loader.h"
#include "emulation.h"
#define CHAR_TO_INT(x) ((x)- '0')



/*
 * @brief load opens a file and reads the contents into memory
 * @param open_file the file to open
 * @param file_name the name of the file to open
 */
void load(FILE *open_file, char *file_name, Emulator *emulator) {
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
        s_record[strcspn(s_record, "\n")] = 0; //clear newline from EOL
        s_record[strcspn(s_record, "\r")] = 0; //clear carriage return from EOL
        if (tolower(s_record[0]) != 's')
        {
            printf("Unexpected value in .xme {%d}, check file input! skipping this line\n", s_record[0]);
            //go to next line
        }
        else
        {
            if (!record_check(s_record))
            {
                //record contains invalid value abort loading
                return;
            }
            else
            {
                //record is good, now we can parse the data
                clean_data(s_record, emulator);

            }
        }
    }
    fclose(open_file);
}
/*
 * @brief record_check checks the record for any unexpected values
 * @param s_record the record to check
 */
bool record_check(char *s_record) {

    for (int i = 1; i < strlen(s_record); ++i)
    {
        if (!isalnum(s_record[i])) //check for non alphanumerics ' ', '?', '\n' etc..
        {
            printf("Unexpected value {%d} in s_record, possibly corrupt value.. Aborting load\n", s_record[i]);
            return false;
        }
        if (toupper(s_record[i]) > 'F' && (s_record[i] != '\n' && s_record[i] != '\0')) //check for the rest of the alphabet
        {
            printf("Unexpected hex character {%c} in s_record possibly corrupt value.. Aborting load\n", s_record[i]);
            return false;
        }
    }
    return true;
}
/*
 * @brief clean_data converts the data from characters into hex values and
 * then calls functions to store the data in memory
 * @param s_record the record to be cleaned
 */
void clean_data(char *s_record, Emulator *emulator) {
    if(s_record == NULL)
    {
        printf("No s_record provided, is NULL\n");
        return;
    }
    unsigned char processed_length;
    int type;
    int processed_address;
    type = CHAR_TO_INT(s_record[TYPE_LOCATION]);

    char raw_length[SIZE_OF_RAW_LENGTH] = {0}; //byte pair version of length
    strncpy(raw_length, s_record + LENGTH_LOCATION, BYTE_SIZE);
    processed_length = (unsigned char) strtol(raw_length, NULL, CONVERT_FROM_HEX);

    char raw_address[SIZE_OF_RAW_ADDRESS] = {0};
    strncpy(raw_address, s_record + ADDRESS_LOCATION, BYTE_SIZE * 2);
    processed_address = (unsigned short) strtol(raw_address, NULL, CONVERT_FROM_HEX);

    char raw_checksum[SIZE_OF_RAW_CHECKSUM] = {0};
    //copy checksum from end of s_record into it's own array
    strncpy(raw_checksum, s_record + (strlen(s_record) - BYTE_SIZE), BYTE_SIZE);
    //create an array of bytes to hold the integer version of our data
    unsigned char *parsed_data = (unsigned char *) calloc(processed_length, sizeof(unsigned char));
    if(parsed_data == NULL)
    {
        printf("Failed when allocating memory for parsed data, aborting load of s_record.\n");
        return;
    }
    //parse the data, array passed by reference because copying is wasteful in this case
    unsigned char sum = 0;
    parse_data(s_record + PREAMBLE, &parsed_data, &sum);
    //verify our data values pair with the checksum provided by the assembler
    if (test_checksum(processed_length, processed_address, raw_checksum, sum))
    {
        store_in_memory(type, processed_address, processed_length, parsed_data, emulator);
    }
    else
    {
        printf("Failed to verify checksum, possibly corrupt file, aborting load of record.\n");
    }
    memset(s_record, '\0', MAX_RECORD_LEN);
    if (parsed_data != NULL)
    {
        free(parsed_data);
        parsed_data = NULL;
    }

}
/*
 * @brief store_in_memory stores according to s_record information
 * @param type the type of s-record
 * @param record_address the address to store the record
 * @param record_length the length of the record
 * @param parsed_data the data to store
 * */
void store_in_memory(int type, int record_address, int record_length, unsigned char *parsed_data, Emulator *emulator) {
    switch(type)
    {
        case 0:
            printf("Loaded File: %s\n", parsed_data);
            break;
        case 1:
            //fall through to case2 as both operations are the same
        case 2:
            memcpy(xm23_memory[type - 1].byte + record_address, parsed_data,
                   record_length);
            printf("S%d Stored\n", type);
            break;
        case 9:
            emulator->starting_address = (short) record_address;
            emulator->reg_file[REGISTER][PROG_COUNTER].word = emulator->starting_address;
            printf("Program starting Addr = %04x\n",
                   emulator->starting_address);
            break;
        default:
            printf("Unknown Type {%d} record not stored\n", type);
            break;
    }
}
/*
 * @brief display_loader_memory displays the loader memory in both raw and ascii
 * it prints on a line by line basis using 16 bytes as the line length so the
 * address increments in +10 HEX each line
 */
void display_loader_memory()
{
    int lower_lookup;
    int upper_lookup;
    char mem_type;
    printf("Enter lower, upper, memory type (I/D)");
    scanf("%4x %4x %c", &lower_lookup, &upper_lookup, &mem_type);
    printf("Entered Memory bounds %4x --> %4x\n", lower_lookup, upper_lookup);

    mem_type = toupper(mem_type) == 'I' ? INSTR : DATA;

    if(lower_lookup < 0 || lower_lookup > BYTE_MEMORY_SIZE)
    {
        printf("Invalid lower bound, clamping to 0\n");
        lower_lookup = 0;
    }
    else if(upper_lookup < 0 || upper_lookup > BYTE_MEMORY_SIZE )
    {
        printf("Invalid upper bound, clamping to 0xFFF\n");
        upper_lookup = 0xFFFF;
    }

    //loop through loader memory from lower to upper bounds and print out the values
    for (int i = lower_lookup; i < upper_lookup; ++i)
    {
        //insert address every 16 bytes
        if (i % MEMORY_LINE_LENGTH == 0)
        {
            printf("%04x: ", i);
        }
        printf("%02x, ", xm23_memory[mem_type].byte[i]);
        if (i > lower_lookup && (i + 1) % MEMORY_LINE_LENGTH == 0)
        {
            //add spaces between the hex and ascii values
            printf("       ");
            //print ascii version of the 16 bytes inserting periods for none ascii characters
            for (int j = i - MEMORY_LINE_LENGTH; j < i; ++j)
            {
                //check if it's a printable character
                if (isprint(xm23_memory[mem_type].byte[j]))
                {
                    printf("%c", xm23_memory[mem_type].byte[j]);
                }
                else
                {
                    printf(".");
                }
            }
            printf("\n");
        }
    }

}
void parse_data(char *string_data, unsigned char **converted_data,
                unsigned char *sum) {

    //check if the entire set of data is missing
    if (string_data == NULL)
    {
        printf("No string passed for function to parse!\n");
        return;
    }
    if(sum == NULL)
    {
        printf("No sum passed for function to modify!\n");
        return;
    }
    else
    {
        //buffer for converting a pair of chars into one int value
        char str_to_convert[BYTE_SIZE + 1];
        //track where we are in the converted data
        int data_index = 0;
        unsigned char val;
        for (int i = 0; i < strlen(string_data) - CHECKSUM_LENGTH; i += BYTE_SIZE)
        {
            //copy 2 chars at a time into the conversion buffer
            strncpy(str_to_convert, string_data + i, BYTE_SIZE);
            //store the into version in the converted buffer
            val = strtol(str_to_convert, NULL, CONVERT_FROM_HEX);
            (*converted_data)[data_index] = val;
            *sum += val;
            data_index++;
        }
    }

}
/*
 * @brief test_checksum tests if the record is valid by adding the values of
 * the s-record from the length byte to the end of the data bytes and the
 * checksum byte. If the sum of the values is equal to 255, the record is valid
 */
bool test_checksum(unsigned char length_byte, unsigned short address_bytes,
                   char *checksum_byte, unsigned char data_sum) {
    unsigned char sum;
    //calculate checksum value from the checksum byte
    unsigned char processed_checksum = strtol(checksum_byte, NULL, CONVERT_FROM_HEX);
    //sum values while extracting upper and lower bytes of address as mentioned in lab
    sum = data_sum + length_byte + (address_bytes & 0xFF) + ((address_bytes >> 8) & 0xFF);
    //check if valid checksum
    if (sum + processed_checksum == VALID_CHECKSUM)
    {
        return true;
    }
    return false;
}
