#include "../inc/CSV_handler.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void write_log_CSV(
    float internal_temp, float external_temp, float ref_temp, double signal,
    char *filepath)
{
  char line[256];

  FILE *log = fopen(filepath, "a");
  for (int i = 0; i < 10; i++) {
    time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  int day = tm.tm_mday, month = tm.tm_mon + 1, year = tm.tm_year + 1900,
      hour = tm.tm_hour, min = tm.tm_min, sec = tm.tm_sec;


  strcpy(line, "\n");
  sprintf(&line[1], "%02d", day);
  strcpy(&line[3], "/");
  sprintf(&line[4], "%02d", month);
  strcpy(&line[6], "/");
  sprintf(&line[7], "%d", year);
  strcpy(&line[11], ",");
  strcpy(&line[12], " ");

  sprintf(&line[13], "%.2f", internal_temp+i);
  strcpy(&line[18], ",");
  strcpy(&line[19], " ");

  sprintf(&line[20], "%.2f", external_temp+(2*i));
  strcpy(&line[25], ",");
  strcpy(&line[26], " ");

  sprintf(&line[27], "%.2f", ref_temp+(3*i));
  strcpy(&line[32], ",");
  strcpy(&line[33], " ");

  sprintf(&line[34], "%.2lf", signal);


  fprintf(log, line);

  }
  fclose(log);

}

int read_curve_CSV(int curr_time, char *filepath)
{
  FILE *curve = fopen(filepath, "r");
  char line[256];
  int prev_time = -1, prev_temp = 0, next_time = 0, next_temp = 0, temperature = -1;

  fgets(line, sizeof(line), curve);

  while(fgets(line, sizeof(line), curve))
    {
      sscanf(line, "%d, %d", &next_time, &next_temp);
      if (curr_time >= prev_time && curr_time < next_time){
        temperature = prev_temp;
        break;
      }
      prev_time = next_time;
      prev_temp = next_temp;
    }
  if (temperature == -1)
    temperature = prev_temp;
  return temperature;

  fclose(curve);
}