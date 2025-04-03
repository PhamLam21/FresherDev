#ifndef __SRECORD_H__
#define __SRECORD_H__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<fcntl.h>

#define MAX_LINE_LENGTH 256
#define ERROR_MSG(line, msg) printf("Error line %d: %s\n", line, msg);
#define CHECK_COND(cond, line, msg) if (cond) { ERROR_MSG(line, msg); return true; }
#define SET_STATE_AND_LEN(type, state_val, len) case type: state = state_val; address_length = len; break;

typedef enum {
    STATE_START,
    STATE_S0,
    STATE_S1_S2_S3,
    STATE_S5,
    STATE_S7_S8_S9,
} State;

/*Function caculate checksum*/
unsigned char caculated_checksum(const char *record);

/*Function check error line*/
bool isError(const char *line, int line_num, int *count_s1_s2_s3);

/*Function print data, address*/
void process_srecord_line(const char *line, int line_num, FILE *output, int *count_s1_s2_s3);

#endif /*__SRECORD_H__*/