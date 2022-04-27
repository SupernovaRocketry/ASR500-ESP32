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
extern char result;

extern String stringDados;

extern BMP280 bmp; 
extern File arquivoLog;
extern SPIClass spi;

extern unsigned long millisGravacao;
extern bool abriuParaquedas;
extern bool abriuRedundancia;
extern double alturaMaxima;
extern double alturaInicial;



void adquireDados() {
    //todas as medidas são feitas aqui em sequeência de maneira que os valores
    //sejam temporalmente próximos

    result = bmp.startMeasurment();

    if(result != 0){
        delay(result);
        result = bmp.getTemperatureAndPressure(temperaturaAtual, pressaoAtual);
        if(result != 0){
            alturaAtual = bmp.altitude(pressaoAtual, PRESSAO_MAR);
            #ifdef DEBUG
                Serial.print("Temperatura Atual: ");
                Serial.println(temperaturaAtual);
                Serial.println("Adquiri os dados");
            #endif
        }
    }    
}

void gravaDados() {
    //verifica aqui o estado do foguete e também se o arquivo está aberto e pronto
    //para ser usado. Aqui, todos os dados são concatenados em uma string que dá
    //o formato das linhas do arquivo de log.
    if ((statusAtual == ESTADO_GRAVANDO) || (statusAtual == ESTADO_RECUPERANDO)) {
        arquivoLog = SD.open(nomeConcat, FILE_APPEND);
        #ifdef DEBUG_TEMP
            Serial.println("Estou gravando!");
            digitalWrite(REC_PRINCIPAL, HIGH);
        #endif
        stringDados = "";
        millisGravacao = millis();
        stringDados += millisGravacao;
        stringDados += ";";
        stringDados += abriuParaquedas;
        stringDados += ";";
        stringDados += abriuRedundancia;
        stringDados += ";";
        stringDados += alturaAtual;
        stringDados += ";";
        stringDados += alturaInicial;
        stringDados += ";";
        stringDados += alturaMaxima;
        stringDados += ";";
        stringDados += pressaoAtual;
        stringDados += ";";
        stringDados += temperaturaAtual;
        stringDados += ";";
        stringDados += statusAtual;


        arquivoLog.println(stringDados);
        arquivoLog.close();
    }

    #ifdef DEBUG
        Serial.println("Gravei os dados");
    #endif
}