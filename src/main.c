#include <stdio.h>
#include <stdlib.h>
#include "../modules/ESP32_communicator.h"
#include <unistd.h>

int main(int argc, char const *argv[])
{
    int uart0_filestream = -1;

    while (1)
    {
        int input = readsUserInput(&uart0_filestream);
        if (input > 0)
        {
            printf("Comando lido: %i\n", input);
            switch (input)
            {
            case 1:
                sendSystemState(&uart0_filestream, 1);
                break;
            case 2:
                sendSystemState(&uart0_filestream, 2);
                break;
            case 3:
                sendControlSignal(&uart0_filestream, 0);
                break;
            case 4:
                sendControlSignal(&uart0_filestream, 1);
                break;
            }
        }

        sleep(1);
    }

    return 0;
}
