#ifndef CSV_HANDLER_H_
#define CSV_HANDLER_H_

int read_curve_CSV(int curr_time, char *filepath);

void write_log_CSV(
    float internal_temp, float external_temp, float ref_temp, double signal,
    char *filepath
);

#endif
