#include <Arduino.h>
#include "BMP280.h"
#include "Wire.h"
#include <SD.h> 
#include <defs.h>
#include <main.h>



extern bool estado;
extern bool gravando;
extern bool subiu;
extern bool descendo;
extern char statusAtual;
extern char erro;
extern char nomeConcat[16];
extern char result;
extern double alturaMaxima;
extern double alturaMinima;
extern double alturaInicial;
extern double alturaAtual;
extern double pressaoAtual;
extern double temperaturaAtual;
extern int o;
extern unsigned long millisAtual;
extern unsigned long millisLed;
extern unsigned long atualizaMillis;
extern BMP280 bmp;
extern File arquivoLog;
extern SPIClass spi;

#ifdef DEBUG_COND
  File arquivoCond;
#endif



void leBotoes() {
  // Funcao responsavel por começar a gravar os dados no cartão SD. 

  millisAtual = millis();
  estado = digitalRead(PINO_BOTAO);

  //Liga a gravação se em espera
  if (!estado && (statusAtual == ESTADO_ESPERA)) {
      statusAtual = ESTADO_GRAVANDO;
  }

  #ifdef DEBUG
    Serial.println("Li os botões");
  #endif  
}


void checaCondicoes() {
  // Funcao responsavel por checar condicoes e atualizar variaveis de extremos
  // (altura maxima, altura minima, etc)

  #ifdef DEBUG_COND
    arquivoCond = SD.open("condicao.txt", FILE_APPEND);
    arquivoCond.print(millisAtual);
  #endif

  if ((statusAtual == ESTADO_GRAVANDO) && !gravando ) {
    alturaMinima = alturaAtual;
    gravando = true;

    #ifdef DEBUG_COND
      arquivoCond.print("a");
    #endif
  }

  //alturaMinima
  if ((alturaAtual < alturaMinima) && (statusAtual == ESTADO_GRAVANDO)){
    alturaMinima = alturaAtual;
    
    #ifdef DEBUG_COND
      arquivoCond.print("b");
    #endif
  }
 

  //alturaMaxima
  if (!subiu && (statusAtual == ESTADO_GRAVANDO)){
    alturaMaxima = 0;

    #ifdef DEBUG_COND
      arquivoCond.print("c");
    #endif
  }

  //controle de subida
  if ((alturaAtual > alturaMinima + THRESHOLD_SUBIDA) && (statusAtual == ESTADO_GRAVANDO) && !subiu ){
    subiu = true;

    #ifdef DEBUG_COND
      arquivoCond.print("d");
    #endif
  }

  //primeira referencia de altura maxima
  if (subiu && (alturaMaxima == 0) && (statusAtual == ESTADO_GRAVANDO)){
    alturaMaxima = alturaAtual;

    #ifdef DEBUG_COND
      arquivoCond.print("e");
    #endif
  }

  //verificar a altura máxima
  if ((alturaAtual > alturaMaxima) && (statusAtual == ESTADO_GRAVANDO) && subiu){
    alturaMaxima =  alturaAtual;

    #ifdef DEBUG_COND
      arquivoCond.print("f");
    #endif
  }

  //Controle de descida, usando um threshold para evitar disparos não
  //intencionais
  if ((alturaAtual + THRESHOLD_DESCIDA < alturaMaxima) && (statusAtual == ESTADO_GRAVANDO) && subiu) {
    descendo = true;
    subiu = false;
    statusAtual = ESTADO_RECUPERANDO;

    #ifdef DEBUG_COND
      arquivoCond.print("g");
    #endif
  }

  #ifdef DEBUG_COND
    arquivoCond.println("");
    arquivoCond.close();
  #endif

  #ifdef DEBUG
    Serial.println("Chequei as condições!");
  #endif
}


void finaliza() {
}


void notifica (char codigo) {
  millisAtual = millis();
  // Funcao para notificar qualquer tipo de problema através do buzzer e leds.

  unsigned int frequencia[10];
  //os tons aqui são tocados por um vetor que contem as frequências. Cada
  //slot do mesmo define um espaço de 100ms.

  #ifdef DEBUG
    Serial.print("Status atual do altímetro:");
    Serial.println(codigo);
  #endif

  switch (codigo) {

    //Problema com o BMP180
    //Ambos os leds piscando juntos + tom de erro
    case ERRO_BMP:
      frequencia[0] = 261;
      frequencia[1] = 261;
      frequencia[2] = 0;
      frequencia[3] = 0;
      frequencia[4] = 220;
      frequencia[5] = 220;
      frequencia[6] = 0;
      frequencia[7] = 0;
      frequencia[8] = 196;
      frequencia[9] = 196;

      if (millisAtual - millisLed > 100) {
        // LED amarelo piscando rapido.
        digitalWrite(PINO_LED_VERD, !digitalRead(PINO_LED_VERD));
        digitalWrite(PINO_LED_VERM, !digitalRead(PINO_LED_VERM));
        digitalWrite(PINO_LED_AZUL, LOW);
        millisLed = millisAtual;
      }

      break;


    //Problema com o SD
    //led vermelho piscando junto com o tom de erro
    case ERRO_SD:
      frequencia[0] = 261;
      frequencia[1] = 261;
      frequencia[2] = 0;
      frequencia[3] = 0;
      frequencia[4] = 220;
      frequencia[5] = 220;
      frequencia[6] = 0;
      frequencia[7] = 0;
      frequencia[8] = 196;
      frequencia[9] = 196;

      if (millisAtual - millisLed > 100) {
        // LED vermelho piscando rapido        
        digitalWrite(PINO_LED_VERM, !digitalRead(PINO_LED_VERM));
        digitalWrite(PINO_LED_AZUL, LOW);
        digitalWrite(PINO_LED_VERD, LOW);
        millisLed = millisAtual;
      }

      break;


    //Estado onde o voo já terminou e faz um tom de recuperação
    //led verde pisca rápido também
    case ESTADO_RECUPERANDO:
      frequencia[0] = 4000;
      frequencia[1] = 4500;
      frequencia[2] = 4000;
      frequencia[3] = 0;
      frequencia[4] = 0;
      frequencia[5] = 0;
      frequencia[6] = 0;
      frequencia[7] = 0;
      frequencia[8] = 0;
      frequencia[9] = 0;

      if (millisAtual - millisLed > 100) {
        // LED verde piscando rapido
        digitalWrite(PINO_LED_VERD, !digitalRead(PINO_LED_VERD));
        digitalWrite(PINO_LED_AZUL, LOW);
        digitalWrite(PINO_LED_VERM, LOW);
        millisLed = millisAtual;
      }

      break;


    //Gravando, pisca um led vermelho como uma câmera e também faz
    //um tom simples.
    case ESTADO_GRAVANDO:
      frequencia[0] = 293;
      frequencia[1] = 293;
      frequencia[2] = 0;
      frequencia[3] = 0;
      frequencia[4] = 0;
      frequencia[5] = 0;
      frequencia[6] = 0;
      frequencia[7] = 0;
      frequencia[8] = 0;
      frequencia[9] = 0;

      if (millisAtual - millisLed > 100) {
        // LED azul piscando rapido
        digitalWrite(PINO_LED_AZUL, !digitalRead(PINO_LED_AZUL));
        digitalWrite(PINO_LED_VERD, LOW);
        digitalWrite(PINO_LED_VERM, LOW);
        millisLed = millisAtual;
      }

      break;

    case ESTADO_ESPERA:
      // LED verde piscando devagar indicando espera
      if (millisAtual - millisLed > 500) {
        digitalWrite(PINO_LED_VERD, !digitalRead(PINO_LED_VERD));
        millisLed = millisAtual;
      }


      break;


  }

  //Lê o vetor de frequencias e toca a frequência na posição atual
  //voltando ao inicio do mesmo quando termina, assim tocando todos os tons

  if (codigo) {
    if (frequencia[o] && (statusAtual != ESTADO_ESPERA)) {
      ledcSetup(0, frequencia[o], 8);
      ledcWrite(0, 127);
    }
    o++;
    if (o > 9)
      o = 0;

  }


#ifdef DEBUG_TEMP
  Serial.print("tocando a posição do vetor:");
  Serial.println(o);

#endif
}


void inicializa() {

  //Inicializando as portas
  pinMode(PINO_BOTAO, INPUT_PULLUP);
  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_LED_VERD, OUTPUT);
  pinMode(PINO_LED_VERM, OUTPUT);
  pinMode(PINO_LED_AZUL, OUTPUT);

  //iniciando recuperação
  pinMode(REC_PRINCIPAL, OUTPUT); //declara o pino do rec principal como output 
  pinMode(REC_SECUNDARIO, OUTPUT); 
  digitalWrite(REC_PRINCIPAL, LOW); //inicializa em baixa 
  digitalWrite(REC_SECUNDARIO, LOW);

  ledcAttachPin(PINO_BUZZER, 0);
  

  // erro = 0;                    // Atribuindo um valor inteiro para um variavel do tipo char
  erro = '\0';

  //Inicializando o Altímetro

  while(!bmp.begin()){
    erro = ERRO_BMP;
    notifica(erro);
  }

  bmp.setOversampling(4);
  
  

  for(int i = 0; i<8; i++){
    result = bmp.startMeasurment();
    delay(result);
    while(result == 0){       // result = 0 significa falha na medição. Outros valores dependem do Oversampling.
      result = bmp.startMeasurment();
      delay(result);   
      erro = ERRO_BMP;
      notifica(erro);
    }
    bmp.getTemperatureAndPressure(temperaturaAtual, pressaoAtual);
    alturaInicial += bmp.altitude(pressaoAtual, PRESSAO_MAR);
  }
  alturaInicial =  alturaInicial*0.125;
 

  //inicializar o cartão SD
  spi = SPIClass(VSPI);
  spi.begin(PINO_SD_SCK,PINO_SD_MISO,PINO_SD_MOSI,PINO_SD_CS);
  while(!SD.begin(PINO_SD_CS, spi)){
    erro = ERRO_SD;
    notifica(erro);
  }


  if (!erro) {
    int n = 1;
    bool parar = false;
    while (!parar)
    {
      #ifdef DEBUG_TEMP
            Serial.println("não deveria estar aqui com o sd ligado");
      #endif
      sprintf(nomeConcat, "/log%d.txt", n);
      if (SD.exists(nomeConcat))
        n++;
      else
        parar = true;
    }

    arquivoLog = SD.open(nomeConcat, FILE_WRITE);
    arquivoLog.println("tempo;paraquedas;altura_atual;altura_inicial;altura_maxima;pressao_atual;temperatura_atual;estado_atual");
    arquivoLog.close();

  #ifdef DEBUG
      Serial.print("Salvando os dados no arquivo ");
      Serial.println(nomeConcat);
  #endif

  }


  if (!erro) {
#ifdef DEBUG
    Serial.println("Nenhum erro iniciando dispositivos, começando o loop do main");
#endif
    statusAtual = ESTADO_ESPERA;
  }

  else {
#ifdef DEBUG
    Serial.print("Altímetro com erro de inicialização código:");
    Serial.println(erro);
#endif
    statusAtual = erro;

    atualizaMillis = millis();
  }


}