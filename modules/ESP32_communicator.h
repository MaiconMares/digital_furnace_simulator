#ifndef ESP32_COMMUNICATOR_H_
#define ESP32_COMMUNICATOR_H_

/*  Explanation: Request internal temperature or potentiometer temperature to ESP32
    Parameters: int *uart0_filestream an integer pointer to file descriptor,
    int temperature_code a code that indicates internal temperature or potentiometer temperature,
    0XC1 or 0XC2, respectively.
*/
float requestTemperature(int *uart0_filestream);

int readsUserInput(int *uart0_filestream);

void sendControlSignal();

void sendReferenceSignal();

int sendSystemState();

int sendControlMode();

#endif
