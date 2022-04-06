#include <Arduino.h>
#include "BMP280.h"
#include "Wire.h"
#include <SD.h>
#include <defs.h>
#include <main.h>


extern double pressaoAtual;
extern double alturaAtual;
extern double temperaturaAtual;

extern char  statusAtual;
extern char nomeConcat[16];

extern String stringDados;

extern BMP280 bmp; 
extern File arquivoLog;

extern unsigned long millisGravacao;
extern bool abriuParaquedas;
extern float alturaMaxima;



void adquireDados() {
    //todas as medidas são feitas aqui em sequeência de maneira que os valores
    //sejam temporalmente próximos
    char result = bmp.getTemperatureAndPressure(temperaturaAtual, pressaoAtual);

    alturaAtual = bmp.altitude(pressaoAtual, PRESSAO_MAR);
}

void gravaDados() {
    //verifica aqui o estado do foguete e também se o arquivo está aberto e pronto
    //para ser usado. Aqui, todos os dados são concatenados em uma string que dá
    //o formato das linhas do arquivo de log.
    if ((statusAtual == ESTADO_GRAVANDO) || (statusAtual == ESTADO_RECUPERANDO)) {
        Serial.println(nomeConcat);
        arquivoLog = SD.open(nomeConcat, FILE_WRITE);
        Serial.println("Linha 42 Dados.cpp");
        #ifdef DEBUG_TEMP
            Serial.println("Estou gravando!");
            digitalWrite(REC_PRINCIPAL, HIGH);
        #endif
        stringDados = "";
        millisGravacao = millis();
        stringDados += millisGravacao;
        stringDados += ",";
        stringDados += abriuParaquedas;
        stringDados += ",";
        stringDados += alturaAtual;
        stringDados += ",";
        stringDados += alturaMaxima;
        stringDados += ",";
        stringDados += pressaoAtual;
        stringDados += ",";
        stringDados += temperaturaAtual;


        arquivoLog.println(stringDados);
        arquivoLog.close();
    }


}