# digital_furnace_simulator

# Trabalho 1 - 2021/2

Trabalho 1 da disciplina de Fundamentos de Sistemas Embarcados (2021/2)

## 1. Objetivos

Este trabalho tem por objetivo a implementação de um sistema (que simula) o controle de um forno para soldagem de placas de circuito impresso (PCBs). Abaixo vemos alguns exemplos de fornos comerciais para este propósito.

Reflow Oven - 1826 Mark5 | Forno LPKF Protoflow S4  |  Controleo3 Reflow Oven
:-------------------------:|:-------------------------:|:-------------------------:
<img src="https://hellerindustries.com/wp-content/uploads/2018/06/1826.png" data-canonical-src="https://hellerindustries.com/wp-content/uploads/2018/06/1826.png" width="200"/> | <img src="https://www.lpkf.com/fileadmin/mediafiles/_processed_/1/b/csm_lpkf_protoflow_s4_05300933fa.png" data-canonical-src="https://www.lpkf.com/fileadmin/mediafiles/_processed_/1/b/csm_lpkf_protoflow_s4_05300933fa.png" width="200"/> | <img src="https://www.whizoo.com/i/c3/c3_oven.jpg" data-canonical-src="https://www.whizoo.com/i/c3/c3_oven.jpg" width="150"/>

No trabalho, o aluno deverá desenvolver o software que efetua o controle de temperatura do forno utilizando dois atuadores para este controle: um resistor de potência de 15 Watts utilizado para aumentar temperatura e; uma ventoinha que puxa o ar externo (temperatura ambiente) para reduzir a temperatura do sistema. 

Os comandos do usuário do sistema para definir a temperatura desejada serão controlados de três maneiras:
1. Através de um Potenciômetro externo;
2. Através de entrada de teclado no terminal;
3. Seguindo uma curva de temperatura pré-definida em arquivo de configuração ([Arquivo da Curva](./curva_reflow.csv)).

O controle da temperatura será realizado através da estratégia PID onde deverão estar disponíveis para o usuário o ajuste dos parâmetros Kp, Ki e Kd nas configurações do sistema (Via terminal).

## 2. Controle PID

A abordagem de controle a ser utilizado é o controle PID (Proporcional Integral Derivativo). O PID é um dos tipos mais simples de algoritmos de controle que proporciona um bom desempenho para uma grande variedade de aplicações.

O conceito fundamental desse tipo de controlador se baseia em monitorar uma variável de um processo (neste caso a temperatura interna) e medir a diferença entre seu valor atual (TI - Temperatura Interna) a uma valor de referência (TR - Temperatura de Referência) desejado. A partir dessa medida de **Erro = TR - TI**, toma-se uma ação de correção para que o sistema alcançe o valor desejado. O figura abaixo demonstra uma visão geral da estratégia de controle.

![Sistema de Controle](https://upload.wikimedia.org/wikipedia/commons/2/24/Feedback_loop_with_descriptions.svg)

O controle PID une três tipos de ações sobre a variável de controle para minimizar o erro do sistema até que o mesmo alcançe a referência desejada. No caso deste sistema, nossa variável monitorada é a TI - Temparatura Interna e o seu controle é efetuado através do acionamento da **Resistência (R)** ou da **Ventoinha (V)** e nosso **Erro** é a diferença entre a temperatura de referência e a temperatura interna do sistema (Erro = TR - TI).

Detalhando as 3 ações do PID temos:
- **Controle Proporcional (P)**: ajusta a variável de controle de forma proporcional ao erro, ou seja, quanto maior o erro, maior a intensidade de acionamento do resistor (0 a 100%). Esse ajuste é feito pela variável ***Kp***.
- **Controle Integral (PI)**: ajusta a variável de controle baseando-se no tempo em que o erro acontece, acumulando este erro (integral). Esse ajuste é feito pela variável ***Ki***.
- **Controle Derivativo (PD)**: ajusta a variável de controle tendo como base a taxa de variação do erro ou a velocidade com a qual o sistema está variando o erro. Esse ajuste é feito pela variável ***Kd***.

A figura abaixo mostra as equações envolvidas no cálculo do PID.

![PID](https://upload.wikimedia.org/wikipedia/commons/4/43/PID_en.svg)

O ajustes de cada constante do PID (Kp, Ki e Kd) tem efeitos distintos no controle do sistema conforme pode ser visto na figura  abaixo.

![PID - Variáveis Kp, Ki, Kd](https://upload.wikimedia.org/wikipedia/commons/3/33/PID_Compensation_Animated.gif)


## 4. Componentes do Sistema

O sistema como um todo é composto por:
1. Ambiente fechado controlado com o resistor de potência e ventoinha;
2. 01 Sensor DS18B20 (1-Wire) para a medição da temperatura interna (TI) do sistema;
3. 01 Sensor BME280 (I2C) para a medição da temperatura externa (TE);
4. 01 módulo Display LCD 16x2 com circuito I2C integrado (Controlador HD44780);
5. 01 Conversor lógico bidirecional (3.3V / 5V);
6. 01 Driver de potência para acionamento de duas cargas;
6. 01 ESP32;
7. 01 Potenciômetro (Para definir a temperatura de referência TR);
8. 01 Switch (Para definir a estratégia de controle);
9. 01 Raspberry Pi 4;

![Figura](/figuras/Figura_Trabalho_1_v2.png)

## 5. Conexões entre os módulos do sistema

1. O sensor de temperatura BM280 está ligado ao barramento I2C e utiliza o endereço (0x76);
2. O módulo de display LCD está conectado ao barramento I2C utilizando o endereço 0x27;
3. O resistor de potência e a ventoinha estão ambos ligados às portas GPIO e são acionados através do circuito de potência;  
    3.1. Resistor: GPIO 23 ou Pino 16 (Board)  
    3.2. Ventoinha: GPIO 24 ou Pino 18 (Board)  
4. A ESP32 está conectada à placa Raspberry Pi via UART (Protocolo MODBUS-RTU);
5. O potenciômetro é conectado à porta analógica (A1) da ESP32;
6. Os comandos de acionamento e controle do sistema virão do Dashboard (Thingsboard) através da ESP32;
7. O sensor de temperatura DS18B20 para medição do ambiente controlado está ligado à ESP32 na porta GPIO 4 via protocolo 1-Wire;

## 6. Requisitos

Os sistema de controle possui os seguintes requisitos:
1. O código deve ser desenvolvido em C/C++;
2. Na implementação do software, não podem haver loops infinitos que ocupem 100% da CPU;
3. O sistema deve implementar o controle de temperatura do ambiente fechado utilizando o controle PID atuando sobre o Resistor e a Ventoinha;
4. A interface de terminal do usuário deve prover a capacidade de definição dos seguintes parâmetros:
   1. **Temperatura de referência (TR)**: deve haver uma opção para escolher se o sistema irá considerar TR definido pelo teclado, pelo potenciômetro via UART ou seguindo as curvas de temperatura definidas em arquivo.
   2. **Parâmetros Kp, Ki, Kd**: para o controle PID deve ser possível definir os valores das constantes Kp, Ki e Kd;
5. No caso da temperatura ser definida via UART (potenciômetro / outros), o programa deve consultar o valor através da comunicação UART com a ESP32 a cada 1 segundo;
6. O programa deve gerar um log em arquivo CSV das seguintes informações a cada 01 segundo com os seguintes valores: (Data e hora, temperatura interna, temperatura externa, temperatura definida pelo usuário, valor de acionamento dos atuadores (Resistor e Venoinha em valor percentual)).
7. **Display LCD (16x2)**: O sistema deve apresentar na tela LCD seu estado atual. Os estados são:
   1. Desligado;
   2. Acionado: mostrar o modo de controle (Terminal ou UART) juntamente com os valores das temperaturas (TI, TE, TR) atualizados a cada 1 segundo;
8.  O programa deve tratar a interrupção do teclado (Ctrl + C = sinal **SIGINT**) encerrando todas as comunicações com periféricos (UART / I2C / GPIO) e desligar os atuadores (Resistor e Ventoinha);
9.  O código deve possuir Makefile para compilação;
10. O sistema deve conter em seu README as instruções de compilação e uso, bem como gráficos* com o resultado de dois experimentos, um com o controle On-Off e outro com o PID sendo executado pelo período de 10 minutos com variação da temperatura de referência sendo dada pelo potenciômetro.
 
\* Serão necessários dois gráficos para cada experimento. Um deles plotando as temperaturas (Ambiente, Interna e Referência (Potenciômetro)) e outro gráfico com o valor do acionamento dos atuadores (Resistor / Ventoinha) em valor percentual entre -100% e 100%.

## 7. Comunicação UART com a ESP32

A comunicação com a ESP32 deve seguir o mesmo protocolo MODBUS utilizado no [Exercício 2](https://gitlab.com/fse_fga/exercicios/exercicio-2-uart-modbus).
 
A ESP32 será responsável por:
1. Efetuar a medição da temperatura interna (Sensor DS18B20);
2. Realizar a leitura do potenciômetro para o controle externo da temperatura de referência;
3. Iniciar e Desligar o sistema de controle;
4. Enviar comandos para a mudança de modo de controle (Potenciômetro ou Curvas de Temperatura);
5. Atualizar informações sobre as temperaturas e o sinal de controle no dashboard (ThingsBoard).

Para acessar as informações via UART envie mensagens em formato MODBUS com o seguinte conteúdo:

1. Código do Dispositivo (ESP32): 0x01
2. Leitura do Valor de Temperatura Interna (TI): Código 0x23, Sub-código: 0xC1 + 4 últimos dígitos da matrícula. O retorno será o valor em Float (4 bytes) da temperatura interna do sistema com o pacote no formato MODBUS;
3. Leitura da temperatura de referência - TR (Potenciômetro): Código 0x23, Sub-código: 0xC2 + 4 últimos dígitos da matrícula. O retorno será o valor em Float (4 bytes) da temperatura de referência definida pelo usuário com o pacote no formato MODBUS;
4. Envio do sinal de controle (Resistor / Ventoinha): Código 0x16,  Sub-código: 0xD1 + 4 últimos dígitos da matrícula, Valor em Int (4 bytes).
5. Envio do sinal de referência nos casos em que o sistema esteja sendo controlado ou pelo terminal ou pela curva de referência: Código 0x16,  Sub-código: 0xD2 + 4 últimos dígitos da matrícula, Valor em Float (4 bytes).
6. Leitura dos Comandos de usuário: Código 0x23,  Sub-código: 0xC3;
7. Envio do estado interno do sistema em resposta aos comandos de usuário:
   1. Estado (Ligado / Desligado): Código 0x16,  Sub-código: 0xD3 + byte;
   2. Modo de Controle (Potenciômetro = 0 / Referência = 1): Código 0x16,  Sub-código: 0xD4 + byte;

<p style="text-align: center;">Tabela 1 - Códigos do Protocolo de Comunicação</p>

| Endereço da ESP32 | Código |	Sub-código + Matricula | Comando de Solicitação de Dados |	Mensagem de Retorno |
|:-:|:-:|:-:|:--|:--|
| **0x01** | **0x23** | **0xC1** N N N N |	Solicita Temperatura Interna  | 0x00 0x23 0xC1 + float (4 bytes) |
| **0x01** | **0x23** | **0xC2** N N N N |	Solicita Temperatura Potenciômetro	| 0x00 0x23 0xC2 + float (4 bytes) |
| **0x01** | **0x23** | **0xC3** N N N N |	Lê comandos do usuário  | 0x00 0x23 0xC3 + int (4 bytes de comando) | 
| **0x01** | **0x16** | **0xD1** N N N N |	Envia sinal de controle Int (4 bytes) | - |
| **0x01** | **0x16** | **0xD2** N N N N |	Envia sinal de Referência Float (4 bytes) | - |
| **0x01** | **0x16** | **0xD3** N N N N |	Envia Estado do Sistema (Ligado = 1 / Desligado = 0) (1 byte) | 0x00 0x16 0xD3 + int (4 bytes de estado) | 
| **0x01** | **0x16** | **0xD4** N N N N |	Modo de Controle (Potenciômetro = 0 / Curva = 1) (1 byte) | 0x00 0x16 0xD4 + int (4 bytes de modo de controle) | 

Obs: todas as mensagens devem ser enviadas com o CRC e também recebidas verificando o CRC. Caso esta verificação não seja válida, a mensagem deverá ser descartada e uma nova solicitação deverá ser realizada.

<p style="text-align: left;">Tabela 2 - Comandos de Usuário via UART</p>

| Comando | Código |
|:--|:-:|
| Liga o Forno | 0x01 |
| Desliga o Forno | 0x02 |
| Aciona modo de controle via potenciômetro | 0x03 |
| Aciona modo de controle via curva de temperatura | 0x04 |

A leitura dos comandos via UART deve ser realizada a cada **1 s**.

## 8. Parâmetros de PID

Para o uso do controle do PID, estão sendo sugeridos os seguintes valores para as constantes:

Placa Rasp42 com Fonte de 12V:
- **Kp** = 30.0
- **Ki** = 0.2
- **Kd** = 400.0
- 
Placa Rasp43 com Fonte de 5V:
- **Kp** = 20.0
- **Ki** = 0.1
- **Kd** = 100.0

Porém, vocês estão livres para testar outros valores que sejam mais adequados.

### Acionamento do Resistor 

O **resistor** deve ser acionado utilizando a técnica de PWM (sugestão de uso da biblioteca WiringPi / SoftPWM). A intensidade de acionamento do resistor por variar entre 0 e 100%.

### Acionamento da Ventoinha

A **venotinha** também deve ser acionada utilizando a técnica de PWM. Porém, há um limite inferior de 40% de intensidade para seu acionamento pelas características do motor elétrico. Ou seja, caso o valor de saída do PID esteja entre 0 e -40%, a ventoinha deverá ser acionada com 40% de sua capacidade.

Observa-se ainda que a saída negativa só indica que o atuador a ser acionado deve ser a ventoinha e não o resistor e o valor de PWM a ser definido deve ser positivo, invertendo o sinal.

## 9. Critérios de Avaliação

A avaliação será realizada seguindo os seguintes critérios:

|   ITEM    |   COMENTÁRIO  |   VALOR   |
|------------------------|---------------------------------------------------------------------------------------------------------|---------|
|**Implementação do controlador PID** | Correta implementação do controlador PID (Resistor / Venotinha), incluindo a leitura das temperaturas e acionamento dos atuadores. |    2,0 |
|**Menu de controle**        | Correta implementação do menu apresentando as informações atualizadas ao usuário e permitindo o controle dos parâmetros do sistema. | 1,0 |
|**Leitura da Temperatura Ambiente**| Leitura dos valores de Temperatura Ambiente (Sensor BME280). | 0,5 |
|**Comunicação UART** | Leitura dos valores de Temperatura Interna, Potenciômetro e Chave e envio do sinal de controle através da comunicação MODBUS-UART. | 2,0 |
|**Mostrador no LCD**        | Apresentação das 3 temperatudas no LCD e estado do sistema. | 1,0 |
|**Armazenamento em arquivo**| Armazenamento em arquivo CSV dos dados medidos. |   0,5 |
|**Qualidade do Código**     | Utilização de boas práticas como o uso de bons nomes, modularização e organização em geral.    |  2,0 |
|**README com Experimento** | Documentação README com instruçoes de compilaçõa, uso e relatório do experimento com o gráfico. |  1,0 |
|**Pontuação Extra**         |   Qualidade e usabilidade acima da média.  |  0,5   |

## 10. Referências

[Controle Liga/Desliga - Wikipedia](https://pt.wikipedia.org/wiki/Controle_liga-desliga)  
[Controle PID - Wikipedia](https://pt.wikipedia.org/wiki/Controlador_proporcional_integral_derivativo)  
[Driver da Bosh para o sensor BME280](https://github.com/BoschSensortec/BME280_driver)  
[Biblioteca BCM2835 - GPIO](http://www.airspayce.com/mikem/bcm2835/)  
[Controle do LCD 16x2 em C](http://www.bristolwatch.com/rpi/i2clcd.htm)  
[Biblioteca WiringPi GPIO](http://wiringpi.com)  
[PWM via WiringPi](https://www.electronicwings.com/raspberry-pi/raspberry-pi-pwm-generation-using-python-and-c)



