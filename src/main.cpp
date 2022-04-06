#include <SPI.h> //biblioteca permite a comunicação pelo protocolo SPI 
#include <SD.h> //biblioteca permite ler e escrever no cartão SD
#include <FS.h>  //SD (File System)
#include <Wire.h> //permite comunicação I2C
#include <Arduino.h> //permite a compreensão do VSCode de que vamos programasr um Arduino
#include "BMP280.h" //permite acesso às funções do BMP280
#include "Wire.h"

// Inclusão dos meus arquivos
#include <defs.h>
#include <recuperacao.h>
#include <estados.h>
#include <dados.h>


//Definições de debug
#define DEBUG
#define DEBUG_TEMP


//Variáveis de bibliotecas, declarando objetos
BMP280 bmp; 
File arquivoLog;

char nomeBase[] = "dataLog"; //não foi utilizada
char nomeConcat[16]; //nome do arquivo

//Variáveis de timing
unsigned long millisAtual   = 0; //atualiza o tempo atual 
unsigned long atualizaMillis = 0; 
unsigned long millisLed   = 0;
unsigned long millisGravacao  = 0;
unsigned long millisRec = 1000000;
int n = 0;
int o =  0;

//Variáveis de dados
double alturaAtual;
double alturaInicial;
double alturaMinima;
double alturaMaxima =  0;
double pressaoAtual;
double temperatura;
double temperaturaAtual;
String stringDados;

//variáveis de controle
bool gravando = false;
bool abriuParaquedas = false;
char erro = false;
char  statusAtual;
bool estado;
bool descendo = false;
bool subiu = false;




void setup() {

#ifdef DEBUG
  Serial.begin(115200);
#endif

#ifdef DEBUG_TEMP
  Serial.begin(115200);

#endif
  //Faz o setup inicial dos sensores de movimento e altura assim
  //como as portas

#ifdef DEBUG
  Serial.println("Iniciando o altímetro");
#endif

  inicializa();

}




void loop() {

  //Recebendo o tempo atual de maneira a ter uma base de tempo
  //para uma taxa de atualização
  millisAtual = millis();

  if ((millis() - millisRec >= TEMPO_RELE) && abriuParaquedas){
    digitalWrite(REC_PRINCIPAL, LOW); //COMENTAR LINHA CASO NÃO FOR NECESSÁRIO 
    digitalWrite(REC_SECUNDARIO, HIGH); //aciona o relé secundário
  }

  if ((millisAtual - atualizaMillis) >= TEMPO_ATUALIZACAO) {
#ifdef DEBUG_TEMP
    Serial.print("Status atual:");
    Serial.println(statusAtual);
    Serial.print("estado atual de erro:");
    Serial.println(erro);
#endif
    //verifica se existem erros e mantém tentando inicializar
    if (erro) {
      inicializa();
      notifica(erro);
    }

    //Se não existem erros no sistema relacionados a inicialização
    //dos dispositivos, fazer:

    if (!erro) {

#ifdef DEBUG
      Serial.println("Rodando o loop de funções");
#endif

      //Verifica os botões e trata o clique simples e o clique longo
      //como controle de início/fim da gravação.
      leBotoes();

#ifdef DEBUG
      Serial.println("Li os botões");
#endif

      //Recebe os dados dos sensores e os deixa salvo em variáveis
      adquireDados();
#ifdef DEBUG
      Serial.println("Adquiri os dados");
#endif

      //Trata os dados, fazendo filtragens e ajustes.
      
#ifdef DEBUG
      Serial.println("Tratei os dados");
#endif

      //Se a gravação estiver ligada, grava os dados.
      gravaDados();
#ifdef DEBUG
      Serial.println("Gravei os dados");
#endif

      //De acordo com os dados recebidos, verifica condições como a
      //altura máxima atingida e seta variáveis de controle de modo
      //que ações consequintes sejam tomadas.
      checaCondicoes();

      //Faz ajustes finais necessários
      finaliza();

      //Caso o voo tenha chegado ao ápice, libera o sistema de recuperação
      recupera();
    }

    //Notifica via LEDs e buzzer problemas com o foguete
    notifica(statusAtual);

    atualizaMillis = millisAtual;
  }

}