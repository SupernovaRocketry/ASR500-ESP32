#include <Arduino.h>

#define PIN_ACIONAMENTO 9

void setup(){
    Serial.begin(115200);
    pinMode(PIN_ACIONAMENTO, OUTPUT);
    delay(20000);

    Serial.print("Acionando o Skib...");
    digitalWrite(PIN_ACIONAMENTO, HIGH);
    delay(200);
    digitalWrite(PIN_ACIONAMENTO, LOW);
    Serial.println("Aionamento finalizado!");
}

void loop(){


}