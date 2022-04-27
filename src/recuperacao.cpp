#include <Arduino.h>
#include <defs.h>
#include <main.h>

extern unsigned long millisRec;
extern bool abriuParaquedas;
extern bool abriuRedundancia;
extern bool descendo;
	
hw_timer_t * timer = NULL;


void IRAM_ATTR redundancia() {
    digitalWrite(REC_PRINCIPAL, LOW); //COMENTAR LINHA CASO NÃO FOR NECESSÁRIO 
    digitalWrite(REC_SECUNDARIO, HIGH); //aciona o relé secundário
    abriuRedundancia = 1;
 
}


void abreParaquedas() {
    // 

    #ifdef DEBUG
    Serial.println("Abrindo o paraquedas!");
    #endif

    digitalWrite(REC_PRINCIPAL, HIGH);
    millisRec = millis(); //armazena o horário que o paraquedas foi aberto
    abriuParaquedas = 1;

    	
    timer = timerBegin(0, 80, true);   
    	
    timerAttachInterrupt(timer, &redundancia, true); 

    timerAlarmWrite(timer, 1500000, false);

    timerAlarmEnable(timer);


}

void recupera () {
    //verifica aqui se o foguete já atingiu o apogeu e se está descendo pelas
    //suas variáveis globais de controle e chama a função que faz o acionamento
    //do paraquedas

    if (descendo && !abriuParaquedas) {
    abreParaquedas();
    }
}