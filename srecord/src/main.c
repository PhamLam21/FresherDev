#include"../inc/Srecord.h"

int main(int argc, char const *argv[]) {
    if(argc != 2) {
        printf("Enter ./main file.txt\n");
        exit(EXIT_FAILURE);
    }
    FILE *input = fopen(argv[1], "r");
    FILE *output = fopen(OUTPUT_FILE, "w");

    if (!input) {
        printf("Can't open input file!\n");
        return 1;
    }
    if(!output) {
        printf("Can't open output file!\n");
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