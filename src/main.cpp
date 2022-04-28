#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>

Adafruit_MPU6050 mpu;
SPIClass spi;
File arquivoLog;
String stringDados;

#define LED_VERMELHO 25
#define LED_VERDE 26
#define LED_AZUL 27
#define PINO_SD_CS 5    //CS VSPI (SD)
#define PINO_SD_SCK 18  //CLK VSPI (SD)
#define PINO_SD_MISO 19 //MISO VSPI (SD)
#define PINO_SD_MOSI 23 //MOSI VSPI (SD)


void setup() {
    pinMode(LED_VERMELHO, OUTPUT);
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_AZUL, OUTPUT);
    digitalWrite(LED_VERMELHO, HIGH);
    Serial.begin(115200);

    // Try to initialize!
    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
        digitalWrite(LED_VERMELHO, HIGH);
        while (1) {
        delay(10);
        }
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    delay(100);

    spi = SPIClass(VSPI);
    spi.begin(PINO_SD_SCK,PINO_SD_MISO,PINO_SD_MOSI,PINO_SD_CS);
    while(!SD.begin(PINO_SD_CS, spi)){
        delay(100);
        digitalWrite(LED_VERMELHO, HIGH);
        digitalWrite(LED_VERDE, HIGH);
    }

    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, HIGH);

    arquivoLog = SD.open("/data.txt", FILE_WRITE);
    arquivoLog.println("tempo;AccelX;AccelY;AccelZ;GyroX;GyroY;GyroZ");
    arquivoLog.close();
}

void loop() {

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

    stringDados = "";
    stringDados += millis();
    stringDados += ";";
    stringDados += a.acceleration.x;
    stringDados += ";";
    stringDados += a.acceleration.y;
    stringDados += ";";
    stringDados += a.acceleration.z;
    stringDados += ";";
    stringDados += g.gyro.x;
    stringDados += ";";
    stringDados += g.gyro.y;
    stringDados += ";";
    stringDados += g.gyro.z;

    arquivoLog = SD.open("/data.txt", FILE_APPEND);
    arquivoLog.println(stringDados);
    arquivoLog.close();
    

  delay(10);
}

// void setup() {
//   // put your setup code here, to run once:
//     spi = SPIClass(VSPI);
//     spi.begin(PINO_SD_SCK,PINO_SD_MISO,PINO_SD_MOSI,PINO_SD_CS);
//     while(!SD.begin(PINO_SD_CS, spi)){
//         delay(100);
//         digitalWrite(LED_VERMELHO, HIGH);
//         digitalWrite(LED_VERDE, HIGH);
//     }

//     digitalWrite(LED_VERMELHO, LOW);
//     digitalWrite(LED_VERDE, HIGH);

//     arquivoLog = SD.open("/data.txt", FILE_WRITE);
//     arquivoLog.println("AccelX;AccelY;AccelZ;GyroX;GyroY;GyroZ");
//     arquivoLog.close();
// }

// void loop() {
//   // put your main code here, to run repeatedly:

// }
