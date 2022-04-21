#ifndef DEFS_H
#define DEFS_H

//Definições de debug
#define DEBUG
// #define DEBUG_TEMP
// #define DEBUG_TH


//Definições de sensores

#define USANDO_BMP180

//Definições default
#define PRESSAO_MAR 1013.25

#define TAMANHO_MEDIA 10 //tamanho do vetor da média móvel
#define SERVO_ABERTO 40
#define SERVO_FECHADO 0

#define TEMPO_RELE 1000 //tempo de atraso entre a abertura do paraquedas principal e do secundário
#define TEMPO_ATUALIZACAO 50 //em milisegundos
#define THRESHOLD_DESCIDA 2   //em metros
#define THRESHOLD_SUBIDA 2  //em metros

//Definições de input, define cada pino para cada variável abaixo
#define PINO_BUZZER 13
#define PINO_BOTAO 34
#define PINO_LED_VERM 25
#define PINO_LED_VERD 27
#define PINO_LED_AZUL 26
#define REC_PRINCIPAL 32
#define REC_SECUNDARIO 33
#define PINO_SD_CS 5    //CS VSPI (SD)
#define PINO_SD_SCK 18  //CLK VSPI (SD)
#define PINO_SD_MISO 19 //MISO VSPI (SD)
#define PINO_SD_MOSI 23 //MOSI VSPI (SD)

//definições de erros
#define ERRO_BMP 'b' //inicializa uma variável de erro para o BMP 

#define ERRO_SD 's' //inicializa uma variável de erro para o leitor SD

//definição de estados
#define ESTADO_GRAVANDO 'g'
#define ESTADO_FINALIZADO 'f'
#define ESTADO_RECUPERANDO 'r'
#define ESTADO_ESPERA 'e'

#endif