#include"Srecord.h"

unsigned char caculated_checksum(const char *record) {
    int size = strlen(record);
    unsigned char sum = 0;
    for(int i = 2; i < size - 2; i += 2) {
        unsigned int byte;
        sscanf(&record[i], "%2x", &byte);
        sum += byte;
    }
    return 0xFF - sum;
}

bool isError(const char *line, int line_num, int *count_s1_s2_s3) {
    int byte_count = 0;
    int record_type = line[1] - '0';
    int length_line = strlen(line);

    CHECK_COND(line[0] != 'S', line_num, "Invalid format!")

    sscanf(&line[2], "%2x", &byte_count);
    int expected_length = (byte_count * 2) + 4;
    CHECK_COND(length_line != expected_length, line_num, "Invalid length!")

    unsigned char expected_checksum;
    sscanf(&line[strlen(line) - 2], "%2x", &expected_checksum);
    CHECK_COND(caculated_checksum(line) != expected_checksum, 
                                    line_num, "Invalid checksum!")
    if(record_type == 5) {
        sscanf(&line[4], "%4x", &byte_count);
        CHECK_COND(byte_count != *count_s1_s2_s3, line_num, "The number of records not matching!")
    }
    
    return false;
}

void process_srecord_line(const char *line, int line_num, FILE *output, int *count_s1_s2_s3) {
    State state = STATE_START;
    int byte_count = 0;
    char address[9] = {0};
    char data[MAX_LINE_LENGTH] = {0};
    int record_type = line[1] - '0';
    int address_length = 0;
    int length_line = strlen(line);

    switch (record_type) {
        SET_STATE_AND_LEN(0, STATE_S0, 4)
        SET_STATE_AND_LEN(1, STATE_S1_S2_S3, 4)
        SET_STATE_AND_LEN(2, STATE_S1_S2_S3, 6)
        SET_STATE_AND_LEN(3, STATE_S1_S2_S3, 8)
        SET_STATE_AND_LEN(5, STATE_S5, 0)
        SET_STATE_AND_LEN(7, STATE_S7_S8_S9, 8)
        SET_STATE_AND_LEN(8, STATE_S7_S8_S9, 6)
        SET_STATE_AND_LEN(9, STATE_S7_S8_S9, 4)
        default:
            ERROR_MSG(line_num, "Record type not found!");
            return;
    }
    
    strncpy(address, &line[4], address_length);

    if (state == STATE_S0) {
        sscanf(&line[2], "%2x", &byte_count);
        if(byte_count == 3) {
            fprintf(output, "%d %s %s\n", line_num, address, "Data not use!");
            return;
        }
        strncpy(data, &line[4 + address_length], length_line - 4 - address_length - 2);
        fprintf(output, "%d %s %s\n", line_num, address, data);
    } else if (state == STATE_S1_S2_S3) {
        (*count_s1_s2_s3)++;
        strncpy(data, &line[4 + address_length], length_line - 4 - address_length - 2);
        fprintf(output, "%d %s %s\n", line_num, address, data);
    } else if (state == STATE_S5) {
        sscanf(&line[4], "%4x", &byte_count);
        fprintf(output, "%d Have %d record\n", line_num, byte_count);
    } else if (state == STATE_S7_S8_S9) {
        fprintf(output, "%d %s %s\n", line_num, address, "Data not use");
    }
}

int main(int argc, char const *argv[]) {
    if(argc != 2) {
        printf("Enter ./main file.txt\n");
        exit(EXIT_FAILURE);
    }
    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen("Output.txt", "w");

    if (!input || !output) {
        printf("Can't open file!\n");
        return 1;
    }
    
    char line[MAX_LINE_LENGTH];
    int line_num = 1;
    int count_s1_s2_s3 = 0;
    
    while (fgets(line, sizeof(line), input)) {
        line[strcspn(line, "\r\n")] = '\0';
        if(!isError(line, line_num, &count_s1_s2_s3)) {
            process_srecord_line(line, line_num, output, &count_s1_s2_s3);
        } 
        line_num++;
    }
    
    fclose(input);
    fclose(output);
    printf("Done! Output in Output.txt\n");
    return 0;
}