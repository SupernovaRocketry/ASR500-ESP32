#include <Arduino.h>
#include <SPI.h> //biblioteca permite a comunicação pelo protocolo SPI 
#include <SD.h> //biblioteca permite ler e escrever no cartão SD
#include <Wire.h> //permite comunicação I2C
//#include "Adafruit_BMP085.h"
#include <Arduino.h> //permite a compreensão do VSCode de que vamos programasr um Arduino

#include "Adafruit_BMP280.h" //permite acesso às funções do BMP280
//Definições de debug
//#define DEBUG
//#define DEBUG_TEMP

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
#define PINO_BUZZER 4
#define PINO_BOTAO 14
#define PINO_LED_VERD 25
#define PINO_LED_VERM 32
#define PINO_LED_AZUL 25
#define REC_PRINCIPAL 26
#define REC_SECUNDARIO 27
#define PINO_SD_CS 5

//definições de erros
#define ERRO_BMP 'b' //inicializa uma variável de erro para o BMP 

#define ERRO_SD 's' //inicializa uma variável de erro para o leitor SD

//definição de estados
#define ESTADO_GRAVANDO 'g'
#define ESTADO_FINALIZADO 'f'
#define ESTADO_RECUPERANDO 'r'
#define ESTADO_ESPERA 'e'

//Variáveis de bibliotecas, declarando objetos
Adafruit_BMP280 bmp; 
File arquivoLog;


char nomeBase[] = "dataLog"; //não foi utilizada
char nomeConcat[12]; //nome do arquivo

//Variáveis de timing
unsigned long millisAtual   = 0; //atualiza o tempo atual 
unsigned long atualizaMillis = 0; 
unsigned long millisLed   = 0;
unsigned long millisGravacao  = 0;
unsigned long millisRec = 1000000;
int n = 0;
int o =  0;


//Variáveis de dados
//int32_t pressaoAtual;
float   alturaAtual;
float   alturaInicial;
float   alturaMinima;
float   alturaMaxima =  0;
float pressaoAtual;
float temperatura;
float temperaturaAtual;
String stringDados;

//variáveis de control
bool gravando = false;
bool  abriuParaquedas = false;
char    erro = false;
char  statusAtual;
bool estado;
bool descendo = false;
bool subiu = false;

//Arrays de som de erro;

void abreParaquedas() {
#ifdef DEBUG
  Serial.println("Abrindo o paraquedas!");
#endif
  digitalWrite(REC_PRINCIPAL, HIGH);
  millisRec = millis(); //armazena o horário que o paraquedas foi aberto
  abriuParaquedas = 1;

}

void leBotoes() {

  millisAtual = millis();
  estado = digitalRead(PINO_BOTAO);

  //Liga a gravação se em espera
  if (estado && (statusAtual == ESTADO_ESPERA)) {
    statusAtual = ESTADO_GRAVANDO;


  }
}

void adquireDados() {

  //todas as medidas são feitas aqui em sequeência de maneira que os valores
  //sejam temporalmente próximos
  pressaoAtual = bmp.readPressure();
  alturaAtual = bmp.readAltitude(PRESSAO_MAR);
  temperaturaAtual = bmp.readTemperature();

}

void gravaDados() {

  //verifica aqui o estado do foguete e também se o arquivo está aberto e pronto
  //para ser usado. Aqui, todos os dados são concatenados em uma string que dá
  //o formato das linhas do arquivo de log.
  if ((statusAtual == ESTADO_GRAVANDO) || (statusAtual == ESTADO_RECUPERANDO)) {
    arquivoLog = SD.open(nomeConcat, FILE_WRITE);
    //  #ifdef DEBUG_TEMP
    //  Serial.println("Estou gravando!");
    //  digitalWrite(REC_PRINCIPAL, HIGH);
    //  #endif
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

void checaCondicoes() {

  if ((statusAtual == ESTADO_GRAVANDO) && !gravando ) {
    alturaMinima = alturaAtual;
    gravando = true;
  }

  //alturaMinima
  if ((alturaAtual < alturaMinima) && (statusAtual == ESTADO_GRAVANDO))
    alturaMinima = alturaAtual;

  //alturaMaxima
  if (!subiu && (statusAtual == ESTADO_GRAVANDO))
    alturaMaxima = 0;

  //controle de subida
  if ((alturaAtual > alturaMinima + THRESHOLD_SUBIDA) && (statusAtual == ESTADO_GRAVANDO) && !subiu )
    subiu = true;

  //primeira referencia de altura maxima
  if (subiu && (alturaMaxima == 0) && (statusAtual == ESTADO_GRAVANDO))
    alturaMaxima = alturaAtual;

  //verificar a altura máxima
  if ((alturaAtual > alturaMaxima) && (statusAtual == ESTADO_GRAVANDO) && subiu)
    alturaMaxima =  alturaAtual;

  //Controle de descida, usando um threshold para evitar disparos não
  //intencionais
  if ((alturaAtual + THRESHOLD_DESCIDA < alturaMaxima) && (statusAtual == ESTADO_GRAVANDO) && subiu) {
    descendo = true;
    subiu = false;
    statusAtual = ESTADO_RECUPERANDO;
  }

}

void finaliza() {
}

void recupera () {

  //verifica aqui se o foguete já atingiu o apogeu e se está descendo pelas
  //suas variáveis globais de controle e chama a função que faz o acionamento
  //do paraquedas
  if (descendo && !abriuParaquedas) {

    abreParaquedas();

  }


}

void notifica (char codigo) {

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
        digitalWrite(PINO_LED_AZUL, !digitalRead(PINO_LED_AZUL));
        // digitalWrite(PINO_LED_VERD, !digitalRead(PINO_LED_VERM));
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
        digitalWrite(PINO_LED_VERM, !digitalRead(PINO_LED_VERM));
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
        digitalWrite(PINO_LED_AZUL, !digitalRead(PINO_LED_AZUL));
        digitalWrite(PINO_LED_VERD, LOW);
        digitalWrite(PINO_LED_VERM, LOW);
        millisLed = millisAtual;
      }

      break;

    case ESTADO_ESPERA:
      //led verde piscando devagar indicando espera
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
      tone(PINO_BUZZER, frequencia[o], TEMPO_ATUALIZACAO);
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
  pinMode(PINO_BOTAO, INPUT);
  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_LED_VERD, OUTPUT);
  pinMode(PINO_LED_VERM, OUTPUT);
  pinMode(PINO_LED_AZUL, OUTPUT);

  //iniciando recuperação
  pinMode(REC_PRINCIPAL, OUTPUT); //declara o pino do rec principal como output 
  pinMode(REC_SECUNDARIO, OUTPUT); 
  digitalWrite(REC_PRINCIPAL, LOW); //inicializa em baixa 
  digitalWrite(REC_SECUNDARIO, LOW);
  erro = 0;

  //Inicializando o Altímetro
  if (!bmp.begin()) {
    erro = ERRO_BMP;
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  alturaInicial =  bmp.readAltitude(PRESSAO_MAR);
  alturaMinima = alturaInicial;


  //inicializar o cartão SD
  if (!SD.begin(PINO_SD_CS)) {

    erro = ERRO_SD;

    return;
  }
  else if (!erro) {
    int n = 1;
    bool parar = false;


    while (!parar)
    {
#ifdef DEBUG_TEMP
      Serial.println("não deveria estar aqui com o sd ligado");
#endif
      sprintf(nomeConcat, "log%d.txt", n);
      if (SD.exists(nomeConcat))
        n++;
      else
        parar = true;
    }

    arquivoLog = SD.open(nomeConcat, FILE_WRITE);
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





