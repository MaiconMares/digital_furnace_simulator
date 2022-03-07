#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <softPwm.h>
#include "../inc/ESP32_communicator.h"
#include "../inc/pid.h"
#include "../inc/LCD_I2C_driver.h"

int PWM_LED1_res = 4;
int PWM_LED2_fan = 5;
int fd = -1;

const double Kp_rasp42 = 30.0;
const double Ki_rasp42 = 0.2;
const double Kd_rasp42 = 400.0;

const double Kp_rasp43 = 20.0;
const double Ki_rasp43 = 0.1;
const double Kd_rasp43 = 100.0;

float terminal_temp = 0.0;

enum user_input
{
    not_read,
    already_read
};

enum user_input user_input_read = not_read;

void define_terminal_mode()
{
    printf("Digite a temperatura de referência (float):\n");
    scanf("%f", &terminal_temp);
    control_mode = terminal;
}

void print_temp_mode_LCD(
    float internal_temp, float refer_temp, float external_temp, enum mode curr_mode)
{
    char firstLine[17], secondLine[17];

    if (curr_mode == potentiometer)
    {
        printf("--------Copiou Potenciômetro!---------\n");
        strcpy(firstLine, "Poten.");
    }
    else if (curr_mode == curve)
    {
        printf("--------Copiou Reflow!---------\n");
        strcpy(firstLine, "Reflow");
    }
    else if (curr_mode == terminal)
    {
        printf("--------Copiou Terminal!---------\n");
        strcpy(firstLine, "Termi.");
    }

    strcpy(&firstLine[6], " TI:");
    sprintf(&firstLine[10], "%.2f", internal_temp);

    strcpy(secondLine, "TR:");
    sprintf(&secondLine[3], "%.2f", refer_temp);

    strcpy(&secondLine[8], " TE:");
    sprintf(&secondLine[12], "%.2f", external_temp);

    ClrLcd();
    lcdLoc(LINE1);
    typeln(firstLine);

    lcdLoc(LINE2);
    typeln(secondLine);
}

void print_sys_state_LCD(enum state curr_state)
{
    char firstLine[17], secondLine[17];

    if (curr_state == on)
    {
        strcpy(firstLine, "Sistema Ligado!");
        ClrLcd();
        lcdLoc(LINE1);
        typeln(firstLine);
    }
    else if (curr_state == off)
    {
        strcpy(firstLine, "Sistema");
        strcpy(secondLine, "Desligado!");

        ClrLcd();
        lcdLoc(LINE1);
        typeln(firstLine);

        lcdLoc(LINE2);
        typeln(secondLine);
    }
}

void activate_actuators(double signal_intensity)
{
    int intensity_led1;
    int intensity_led2;
    int uart0_filestream = -1;

    int actuator_intensity = signal_intensity;

    if (actuator_intensity >= -100 && actuator_intensity < 0)
    {
        intensity_led2 = actuator_intensity * -1;
        if (actuator_intensity > -40 && actuator_intensity < 0)
        {
            softPwmWrite(PWM_LED2_fan, 40);
            sendControlSignal(&uart0_filestream, -40);
        }
        else
        {
            softPwmWrite(PWM_LED2_fan, intensity_led2);
            sendControlSignal(&uart0_filestream, actuator_intensity);
        }
    }
    else if (actuator_intensity > 0 && actuator_intensity <= 100)
    {
        intensity_led1 = actuator_intensity;
        softPwmWrite(PWM_LED1_res, intensity_led1);
        sendControlSignal(&uart0_filestream, intensity_led1);
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
            define_terminal_mode();
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
        printf("2-Utilizar valores padrão Rasp43\n");
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
    int uart0_filestream = -1, comm = -1;

    wiringPiSetup();

    pinMode(PWM_LED1_res, OUTPUT);
    pinMode(PWM_LED2_fan, OUTPUT);
    softPwmCreate(PWM_LED1_res, 1, 100);
    softPwmCreate(PWM_LED2_fan, 1, 100);

    fd = wiringPiI2CSetup(I2C_ADDR);
    lcd_init(fd);

    sendSystemState(&uart0_filestream, 0);
    sendControlMode(&uart0_filestream, 0);
    menu(control_mode);

    while (1)
    {
        if (user_input_read == not_read)
            comm = readsUserInput(&uart0_filestream);

        printf("Comando atual: %d\n", comm);
        printf("Estado atual: %d\n", sys_state);
        if (comm == 1 && sys_state == on)
        {
            sendSystemState(&uart0_filestream, on);
            print_sys_state_LCD(on);
        }
        else if (comm == 2 && sys_state == off)
        {
            sendSystemState(&uart0_filestream, off);

            pinMode(PWM_LED1_res, INPUT);
            pinMode(PWM_LED2_fan, INPUT);

            print_sys_state_LCD(off);
            user_input_read = not_read;
        }
        else if (comm == 3 && (control_mode == potentiometer))
        {
            printf("Potenciômetro tentativa!\n");
            if (sys_state == on)
            {
                sendControlMode(&uart0_filestream, (int)potentiometer);
                while (1)
                {
                    float internalTemp = requestTemperature(&uart0_filestream, 193);
                    float potenTemp = requestTemperature(&uart0_filestream, 194);

                    printf("Temperatura interna: %f\n", internalTemp);
                    printf("Temperatura potenciômetro: %f\n", potenTemp);

                    pid_atualiza_referencia(potenTemp);
                    double signal = pid_controle((double)internalTemp);
                    activate_actuators(signal);

                    print_temp_mode_LCD(internalTemp, potenTemp, 0.0, potentiometer);

                    //Printar no CSV
                    comm = readsUserInput(&uart0_filestream);
                    if (comm == 4)
                    {
                        user_input_read = already_read;
                        control_mode = curve;
                        break;
                    }
                    else if (comm == 3)
                    {
                        sendControlMode(&uart0_filestream, (int)potentiometer);
                    }
                    else if (comm == 2)
                    {
                        user_input_read = already_read;
                        sys_state = off;
                        break;
                    }
                }
            }
        }
        else if (comm == 4 && (control_mode == curve))
        {
            printf("Curva tentativa!\n");
            if (sys_state == on)
            {
                sendControlMode(&uart0_filestream, (int)curve);
                while (1)
                {
                    printf("Lendo a partir da curva...\n");

                    float internalTemp = requestTemperature(&uart0_filestream, 193);

                    //Ler temperatura do arquivo
                    printf("Temperatura interna: %f\n", internalTemp);
                    //printf("Temperatura curva: %f\n", potenTemp);

                    sendReferenceSignal(&uart0_filestream, 0.0);

                    //pid_atualiza_referencia(potenTemp);
                    //double signal = pid_controle((double)internalTemp);
                    //activate_actuators(signal);

                    print_temp_mode_LCD(internalTemp, 0.0, 0.0, curve);

                    comm = readsUserInput(&uart0_filestream);
                    if (comm == 4)
                    {
                        sendControlMode(&uart0_filestream, (int)curve);
                    }
                    else if (comm == 3)
                    {
                        user_input_read = already_read;
                        control_mode = potentiometer;
                        break;
                    }
                    else if (comm == 2)
                    {
                        user_input_read = already_read;
                        sys_state = off;
                        break;
                    }
                    sleep(1);
                }

                //Começar a partir da temperatura externa
            }
        }
        else if (control_mode == terminal)
        {
            printf("Terminal tentativa!\n");
            if (sys_state == on)
            {
                sendControlMode(&uart0_filestream, (int)terminal);
                while (1)
                {
                    printf("Lendo a partir do terminal...\n");

                    float internalTemp = requestTemperature(&uart0_filestream, 193);

                    //Ler temperatura do arquivo
                    printf("Temperatura interna: %f\n", internalTemp);
                    printf("Temperatura terminal: %f\n", terminal_temp);

                    sendReferenceSignal(&uart0_filestream, 0.0);
                    pid_atualiza_referencia(terminal_temp);
                    double signal = pid_controle((double)internalTemp);
                    activate_actuators(signal);

                    print_temp_mode_LCD(internalTemp, terminal_temp, 0.0, terminal);

                    comm = readsUserInput(&uart0_filestream);
                    if (comm == 4)
                    {
                        user_input_read = already_read;
                        control_mode = curve;
                        break;
                    }
                    else if (comm == 3)
                    {
                        user_input_read = already_read;
                        control_mode = potentiometer;
                        break;
                    }
                    else if (comm == 2)
                    {
                        user_input_read = already_read;
                        sys_state = off;
                        break;
                    }
                    sleep(1);
                }

                //Começar a partir da temperatura externa
            }
        }
        sleep(1);
    }
    return 0;
}
