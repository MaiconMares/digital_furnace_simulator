#ifndef CSV_HANDLER_H_
#define CSV_HANDLER_H_

float readCSV(char *filepath);

void writeCSV(
    char *date, char *time, char *temp_ext, char *temp_int, char *temp_ref,
    float resistor_value, float fan_value
);

#endif
