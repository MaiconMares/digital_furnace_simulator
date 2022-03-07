#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <softPwm.h>
#include "../inc/ESP32_communicator.h"
#include "../inc/pid.h"

static int PWM_LED1_res = 4;
static int PWM_LED2_fan = 5;

const double Kp_rasp42 = 30.0;
const double Ki_rasp42 = 0.2;
const double Kd_rasp42 = 400.0;

const double Kp_rasp43 = 20.0;
const double Ki_rasp43 = 0.1;
const double Kd_rasp43 = 100.0;

void activate_actuators(double signal_intensity)
{
    int intensity_led1;
    int intensity_led2;

    int actuator_intensity = signal_intensity;

    if (actuator_intensity >= -100 && actuator_intensity < 0)
    {
        intensity_led1 = actuator_intensity;
        if (actuator_intensity > -40 && actuator_intensity < 0)
            softPwmWrite(PWM_LED2_fan, -40);
        else
            softPwmWrite(PWM_LED2_fan, intensity_led1);

    } else if (actuator_intensity > 0 && actuator_intensity <= 100)
    {
        intensity_led2 = actuator_intensity;
        softPwmWrite(PWM_LED1_res, intensity_led2);
    }
}

void user_consts_PID()
{
    double Kp_user = 0.0, Ki_user = 0.0, Kd_user = 0.0;

    printf("Digite o valor de Kp: \n");
    scanf("%lf", &Kp_user);

    printf("Digite o valor de Ki: \n");
    scanf("%lf", &Ki_user);

    printf("Digite o valor de Kd: \n");
    scanf("%lf", &Kd_user);

    pid_configura_constantes(Kp_rasp43, Ki_rasp43, Kd_rasp43);
}

void menu(enum mode control_mode)
{
    int opt = -1;

    wiringPiSetup();
    pinMode(PWM_LED1_res, OUTPUT);
    pinMode(PWM_LED2_fan, OUTPUT);
    softPwmCreate(PWM_LED1_res, 1, 100);
    softPwmCreate(PWM_LED2_fan, 1, 100);

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

    opt = -1;

    while (opt == -1)
    {
        printf("================CONFIGURAÇÃO INICIAL================\n");
        printf("Definir valores para constantes PID:\n");
        printf("1-Utilizar valores padrão Rasp42\n");
        printf("2-Utilizar valores padrão Rasp42\n");
        printf("3-Definir meus próprios valores\n");
        scanf("%d", &opt);

        switch (opt)
        {
        case 1:
            pid_configura_constantes(Kp_rasp42, Ki_rasp42, Kd_rasp42);
            break;
        case 2:
            pid_configura_constantes(Kp_rasp43, Ki_rasp43, Kd_rasp43);
            break;
        case 3:
            user_consts_PID();
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
            sendSystemState(&uart0_filestream, on);
            //sendControlMode(&uart0_filestream, (int) potentiometer);
        }
        else if (comm == 2 && sys_state == off)
        {
            sendSystemState(&uart0_filestream, off);
            //Implementar desligar ventoinha e resistência
        }
        else if (comm == 3 && (control_mode == potentiometer))
        {
            printf("Potenciômetro tentativa!\n");
            if (sys_state == on)
            {
                int status = sendControlMode(&uart0_filestream, (int)potentiometer);
                float internalTemp = requestTemperature(&uart0_filestream, 193);
                float potenTemp = requestTemperature(&uart0_filestream, 194);
                printf("Temperatura interna: %f\n", internalTemp);
                printf("Temperatura potenciômetro: %f\n", potenTemp);
                pid_atualiza_referencia(potenTemp);
                double signal = pid_controle((double)internalTemp);
                activate_actuators(signal);
            }
        }
        else if (comm == 4 && (control_mode == curve))
        {
            printf("Curva tentativa!\n");
            if (sys_state == on)
            {
                if (sendControlMode(&uart0_filestream, (int)curve) == 4)
                {
                    float internalTemp = requestTemperature(&uart0_filestream, 193);
                    printf("Temperatura interna: %f\n", internalTemp);

                    //Ler temperatura interna
                    //Ler temperatura do arquivo
                    //Mandar para o PID e acionar os atuadores
                    //Começar a partir da temperatura externa
                    printf("Enviou curva\n");
                }
            }
        }
        sleep(1);
    }

    return 0;
}
