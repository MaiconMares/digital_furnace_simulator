#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include "../inc/ESP32_communicator.h"

enum state sys_state = off;
enum mode control_mode = potentiometer;

const char matricula[] = {3, 4, 1, 1};

void initUART(int *uart0_filestream)
{
    *uart0_filestream = -1;

    *uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (*uart0_filestream == -1)
        printf("Erro - não foi possível iniciar UART\n");
    else
        printf("UART inicializada!\n");

    struct termios options;
    tcgetattr(*uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(*uart0_filestream, TCIFLUSH);
    tcsetattr(*uart0_filestream, TCSANOW, &options);
}

float requestTemperature(int *uart0_filestream, int temperature_code)
{
    int value = 0;
    unsigned char tx_buffer[8];
    short crc = 0;

    tx_buffer[0] = 1;
    tx_buffer[1] = 35;
    tx_buffer[2] = temperature_code;
    tx_buffer[3] = 3;
    tx_buffer[4] = 4;
    tx_buffer[5] = 1;
    tx_buffer[6] = 1;

    initUART(uart0_filestream);
    printf("Inicializou UART\n");
    if (*uart0_filestream != -1)
    {
        printf("Escrevendo dados na UART...\n");
        int count = write(*uart0_filestream, tx_buffer, 7);
        if (count < 0)
        {
            printf("UART TX error\n");
            close(*uart0_filestream);
            return -1.0;
        }
        else
            printf("Escrito!\n");
    }

    sleep(1);

    if (*uart0_filestream != -1)
    {
        unsigned char rx_buffer[255];

        int rx_length = read(*uart0_filestream, (void *)rx_buffer, 255);
        float data = -1;
        short crcReceived = 0;

        memcpy(&data, &rx_buffer[3], 4);

        if (rx_length < 0)
        {
            printf("Erro na leitura.\n");
            close(*uart0_filestream);
            return -1.0;
        }
        else
        {
            printf("======Dados recebidos:======\n");
            printf("Endereço do dispositivo: %x\n", rx_buffer[0]);
            printf("Código da função: %x\n", rx_buffer[1]);
            printf("Comando de envio: %x\n", rx_buffer[2]);
            printf("%i bytes recebidos: %f\n", rx_length, data);

            close(*uart0_filestream);
            return data;
        }
    }
}

int readsUserInput(int *uart0_filestream)
{
    unsigned char tx_buffer[9];
    short crc = 0;

    printf("----------readsUserInput()----------\n");

    tx_buffer[0] = 1;
    tx_buffer[1] = 35;
    tx_buffer[2] = 195;
    tx_buffer[3] = 3;
    tx_buffer[4] = 4;
    tx_buffer[5] = 1;
    tx_buffer[6] = 1;

    crc = calcula_CRC(tx_buffer, 7);
    memcpy(&tx_buffer[7], &crc, 2);

    initUART(uart0_filestream);
    printf("Inicializou UART\n");
    if (*uart0_filestream != -1)
    {
        printf("Escrevendo dados na UART...\n");
        int count = write(*uart0_filestream, tx_buffer, 9);
        if (count < 0)
            printf("UART TX error\n");
        else
        {
            printf("Escrito!\n");
            printf("CRC Enviado: %d\n", crc);
        }
    }

    sleep(1);

    if (*uart0_filestream != -1)
    {
        unsigned char rx_buffer[255];

        int rx_length = read(*uart0_filestream, (void *)rx_buffer, 255);
        int data = -1;
        short crcReceived = 0;

        memcpy(&data, &rx_buffer[3], 4);
        memcpy(&crcReceived, &rx_buffer[7], 2);

        if (rx_length < 0)
        {
            printf("Erro na leitura.\n");

            close(*uart0_filestream);
            return -1;
        }
        else
        {
            if (data == 1)
                sys_state = on;
            else if (data == 2)
                sys_state = off;
            else if (data == 3)
                control_mode = potentiometer;
            else if (data == 4)
                control_mode = curve;

            printf("======Dados recebidos:======\n");
            printf("Endereço do dispositivo: %x\n", rx_buffer[0]);
            printf("Código da função: %x\n", rx_buffer[1]);
            printf("Comando de envio: %x\n", rx_buffer[2]);
            printf("%i bytes recebidos: %d\n", rx_length, data);

            printf("CRC Recebido: %hi\n", crcReceived);
            printf("Comparando CRC's...\n");

            short crcValidated = calcula_CRC(rx_buffer, 7);
            printf("CRC calculado: %hi\n", crcValidated);

            if (crcValidated != crcReceived)
                printf("CRC's diferentes. Por favor, informe os dados novamente!\n");
            else
                printf("O CRC recebido é válido!\n");

            close(*uart0_filestream);
            return data;
        }
    }
}

void sendControlSignal(int *uart0_filestream, int signal)
{
    unsigned char tx_buffer[11];
    short crc = 0;

    tx_buffer[0] = 1;
    tx_buffer[1] = 22;
    tx_buffer[2] = 209;
    tx_buffer[3] = 3;
    tx_buffer[4] = 4;
    tx_buffer[5] = 1;
    tx_buffer[6] = 1;

    memcpy(&tx_buffer[7], &signal, 4);

    initUART(uart0_filestream);
    printf("Inicializou UART\n");
    if (*uart0_filestream != -1)
    {
        printf("Escrevendo dados na UART...\n");
        int count = write(*uart0_filestream, tx_buffer, 7);
        if (count < 0)
            printf("UART TX error\n");
        else
            printf("Escrito!\n");
    }

    close(*uart0_filestream);
}

void referenceSignal(int *uart0_filestream, float signal)
{
    unsigned char tx_buffer[11];
    short crc = 0;

    tx_buffer[0] = 1;
    tx_buffer[1] = 22;
    tx_buffer[2] = 210;
    tx_buffer[3] = 3;
    tx_buffer[4] = 4;
    tx_buffer[5] = 1;
    tx_buffer[6] = 1;

    memcpy(&tx_buffer[7], &signal, 4);

    initUART(uart0_filestream);
    printf("Inicializou UART\n");
    if (*uart0_filestream != -1)
    {
        printf("Escrevendo dados na UART...\n");
        int count = write(*uart0_filestream, tx_buffer, 7);
        if (count < 0)
            printf("UART TX error\n");
        else
            printf("Escrito!\n");
    }

    close(*uart0_filestream);
}

int sendSystemState(int *uart0_filestream, int state)
{
    unsigned char tx_buffer[10];
    short crc = 0;

    tx_buffer[0] = 1;
    tx_buffer[1] = 22;
    tx_buffer[2] = 211;
    tx_buffer[3] = 3;
    tx_buffer[4] = 4;
    tx_buffer[5] = 1;
    tx_buffer[6] = 1;
    tx_buffer[7] = state;

    crc = calcula_CRC(tx_buffer, 8);
    memcpy(&tx_buffer[8], &crc, 2);

    printf("----------sendSystemState()----------\n");
    initUART(uart0_filestream);
    printf("Inicializou UART\n");
    if (*uart0_filestream != -1)
    {
        printf("Escrevendo dados na UART...\n");
        int count = write(*uart0_filestream, tx_buffer, 10);
        if (count < 0)
        {
            printf("UART TX error\n");
            close(*uart0_filestream);
            return -1;
        }
        else
        {
            printf("Escrito!\n");
            printf("CRC Enviado: %d\n", crc);
        }
    }

    sleep(1);

    if (*uart0_filestream != -1)
    {
        unsigned char rx_buffer[255];

        int rx_length = read(*uart0_filestream, (void *)rx_buffer, 255);
        int data = -1;
        short crcReceived = 0;

        memcpy(&data, &rx_buffer[3], 4);
        memcpy(&crcReceived, &rx_buffer[7], 2);

        if (rx_length < 0)
        {
            printf("Erro na leitura.\n");

            close(*uart0_filestream);
            return -1;
        }
        else
        {
            if (data == 1)
                printf("Liga forno\n");
            else if (data == 2)
                printf("Desliga forno\n");

            printf("======Dados recebidos:======\n");
            printf("Endereço do dispositivo: %x\n", rx_buffer[0]);
            printf("Código da função: %x\n", rx_buffer[1]);
            printf("Comando de envio: %x\n", rx_buffer[2]);
            printf("%i bytes recebidos: %d\n", rx_length, data);

            printf("CRC Recebido: %hi\n", crcReceived);
            printf("Comparando CRC's...\n");

            short crcValidated = calcula_CRC(rx_buffer, 7);
            printf("CRC calculado: %hi\n", crcValidated);

            if (crcValidated != crcReceived)
                printf("CRC's diferentes. Por favor, informe os dados novamente!\n");
            else
                printf("O CRC recebido é válido!\n");

            close(*uart0_filestream);
            return data;
        }
    }
}

int sendControlMode(int *uart0_filestream, int mode)
{
    unsigned char tx_buffer[10];
    short crc = 0;

    printf("----------sendControlMode()----------\n");

    tx_buffer[0] = 1;
    tx_buffer[1] = 22;
    tx_buffer[2] = 212;
    tx_buffer[3] = 3;
    tx_buffer[4] = 4;
    tx_buffer[5] = 1;
    tx_buffer[6] = 1;
    tx_buffer[7] = mode;

    crc = calcula_CRC(tx_buffer, 8);
    memcpy(&tx_buffer[8], &crc, 2);

    initUART(uart0_filestream);
    printf("Inicializou UART\n");
    if (*uart0_filestream != -1)
    {
        printf("Escrevendo dados na UART...\n");
        int count = write(*uart0_filestream, tx_buffer, 10);
        if (count < 0)
        {
            printf("UART TX error\n");
            close(*uart0_filestream);
            return -1;
        }
        else
            printf("Escrito!\n");
    }

    sleep(1);

    if (*uart0_filestream != -1)
    {
        unsigned char rx_buffer[255];

        int rx_length = read(*uart0_filestream, (void *)rx_buffer, 255);
        int data = -1;
        short crcReceived = 0;

        memcpy(&data, &rx_buffer[3], 4);

        if (rx_length < 0)
        {
            printf("Erro na leitura.\n");
            close(*uart0_filestream);
            return -1;
        }
        else
        {
            printf("======Dados recebidos:======\n");
            printf("Endereço do dispositivo: %x\n", rx_buffer[0]);
            printf("Código da função: %x\n", rx_buffer[1]);
            printf("Comando de envio: %x\n", rx_buffer[2]);
            printf("%i bytes recebidos: %i\n", rx_length, data);

            printf("CRC Recebido: %hi\n", crcReceived);
            printf("Comparando CRC's...\n");

            short crcValidated = calcula_CRC(rx_buffer, 7);
            printf("CRC calculado: %hi\n", crcValidated);

            if (crcValidated != crcReceived)
                printf("CRC's diferentes. Por favor, informe os dados novamente!\n");
            else
                printf("O CRC recebido é válido!\n");

            close(*uart0_filestream);
            return data;
        }
    }
}
