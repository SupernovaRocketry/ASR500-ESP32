#include <Arduino.h>
#include <defs.h>
#include <main.h>

extern unsigned long millisRec;
extern bool abriuParaquedas;
extern bool descendo;


void abreParaquedas() {
    // 

    #ifdef DEBUG
    Serial.println("Abrindo o paraquedas!");
    #endif

    digitalWrite(REC_PRINCIPAL, HIGH);
    millisRec = millis(); //armazena o horário que o paraquedas foi aberto
    abriuParaquedas = 1;
}

void recupera () {
    //verifica aqui se o foguete já atingiu o apogeu e se está descendo pelas
    //suas variáveis globais de controle e chama a função que faz o acionamento
    //do paraquedas

    if (descendo && !abriuParaquedas) {
    abreParaquedas();
    }
}