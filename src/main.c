#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../inc/ESP32_communicator.h"

void menu(enum mode control_mode)
{
    int opt = -1;
    while (opt == -1)
    {
        printf("================CONFIGURAÇÃO INICIAL================\n");
        printf("Escolha uma das opções a seguir:\n");
        printf("1-Acionar modo de controle via potenciômetro\n");
        printf("2-Acionar modo de controle via curva de temperatura\n");
        printf("3-Acionar modo de controle via terminal\n");
        printf("4-Sair\n");
        scanf("%d", &opt);
        switch (opt)
        {
        case 1:
            control_mode = potentiometer;
            break;
        case 2:
            control_mode = curve;
            break;
        case 3:
            control_mode = terminal;
            break;
        case 4:
            exit(0);
            break;
        default:
            printf("Opção inválida!\n");
            break;
        }
    }
}

int main(int argc, char const *argv[])
{
    int uart0_filestream = -1;

    sendSystemState(&uart0_filestream, 0);
    sendControlMode(&uart0_filestream, 0);
    menu(control_mode);
    while (1)
    {
        int comm = readsUserInput(&uart0_filestream);
        printf("Comando atual: %d\n", comm);
        printf("Estado atual: %d\n", sys_state);
        if (comm == 1 && sys_state == on)
        {
            printf("Enviou ligar!\n");
            sys_state = on;
            sendSystemState(&uart0_filestream, on);
            sendControlMode(&uart0_filestream, control_mode);
        }
        else if (comm == 2 && sys_state == off)
        {
            printf("Enviou desligar!\n");
            sys_state = off;
            sendSystemState(&uart0_filestream, off);
            //Implementar desligar ventoinha e resistência
        }
        else if (comm == 3 && (control_mode == potentiometer)) {
            control_mode = potentiometer;

            printf("Potenciômetro tentativa!\n");
            if (sys_state == on) {
                if (sendControlMode(&uart0_filestream, potentiometer) == 3)
                    printf("Enviou potenciômetro\n");
            }
        }
        else if (comm == 4 && (control_mode == curve)) {
            control_mode = curve;

            printf("Curva tentativa!\n");
            if (sys_state == on) {
                if (sendControlMode(&uart0_filestream, curve) == 4)
                    printf("Enviou curva\n");
            }
        }
        sleep(1);
    }

    return 0;
}
