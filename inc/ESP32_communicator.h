#ifndef ESP32_COMMUNICATOR_H_
#define ESP32_COMMUNICATOR_H_


void initUART(int *uart0_filestream);

/*  Explanation: Request internal temperature or potentiometer temperature to ESP32
    Parameters: int *uart0_filestream an integer pointer to file descriptor,
    int temperature_code a code that indicates internal temperature or potentiometer temperature,
    0XC1 or 0XC2, respectively.
*/
float requestTemperature(int *uart0_filestream, int temperature_code);

int readsUserInput(int *uart0_filestream);

void sendControlSignal(int *uart0_filestream, int signal);

void sendReferenceSignal(int *uart0_filestream, float signal);

int sendSystemState(int *uart0_filestream, int state);

int sendControlMode(int *uart0_filestream, int mode);

#endif
