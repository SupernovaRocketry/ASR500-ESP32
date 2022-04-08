#include <SD.h>      //biblioteca permite ler e escrever no cartão SD
#include <Wire.h>    //permite comunicação I2C
#include <Arduino.h> //permite a compreensão do VSCode de que vamos programasr um Arduino
#include <BMP280.h>  //permite acesso às funções do BMP280

#define DEBUG

// Definições de sensores
#define USANDO_BMP180

// Definições default
#define PRESSAO_MAR 1013.25
#define THRESHOLD_DESCIDA 2 // em metros
#define THRESHOLD_SUBIDA 2  // em metros

// Definições de input, define cada pino para cada variável abaixo
#define PINO_BUZZER 4
#define PINO_BOTAO 14
#define PINO_LED_VERD 25
#define PINO_LED_VERM 32
#define PINO_LED_AZUL 33
#define REC_PRINCIPAL 26
#define REC_SECUNDARIO 27
#define PINO_SD_CS 5

// definições de erros
#define ERRO_BMP 'b' // inicializa uma variável de erro para o BMP
#define ERRO_SD 's'  // inicializa uma variável de erro para o leitor SD

// definição de estados
#define ESTADO_GRAVANDO 'g'
#define ESTADO_FINALIZADO 'f'
#define ESTADO_RECUPERANDO 'r'
#define ESTADO_ESPERA 'e'

// Variáveis de bibliotecas, declarando objetos
BMP280 bmp;
File arquivoLog;

char nomeConcat[16]; // nome do arquivo

// Variáveis de dados
double alturaAtual;
double alturaInicial;
double alturaMinima;
double alturaMaxima = 0;
double pressaoAtual;
double temperatura;
double temperaturaAtual;
unsigned long millisRec;

// variáveis de controle
bool abriuParaquedas = false;
char statusAtual;
bool descendo = false;
bool subiu = false;

// SemaphoreHandle_t xMutex;

TaskHandle_t notifica;
TaskHandle_t adquireDados;
TaskHandle_t gravaDados;
TaskHandle_t recupera;

void IRAM_ATTR botaoPressionado()
{
    if (statusAtual == ESTADO_ESPERA)
    {
        statusAtual = ESTADO_GRAVANDO;
    }
}

void inicializa()
{
    // Inicializando as portas
    pinMode(PINO_BOTAO, INPUT_PULLUP);
    pinMode(PINO_BUZZER, OUTPUT);
    pinMode(PINO_LED_VERD, OUTPUT);
    pinMode(PINO_LED_VERM, OUTPUT);
    pinMode(PINO_LED_AZUL, OUTPUT);
    // iniciando recuperação
    pinMode(REC_PRINCIPAL, OUTPUT); // declara o pino do rec principal como output
    pinMode(REC_SECUNDARIO, OUTPUT);
    digitalWrite(REC_PRINCIPAL, LOW); // inicializa em baixa
    digitalWrite(REC_SECUNDARIO, LOW);

    // interrupt
    attachInterrupt(digitalPinToInterrupt(PINO_BOTAO), botaoPressionado, FALLING);

    char erro = 0;

    // Inicializando o Altímetro
    if (!bmp.begin())
    {
        erro = ERRO_BMP;
    }
    bmp.setOversampling(4);
    bmp.getTemperatureAndPressure(temperatura, pressaoAtual);
    alturaInicial = bmp.altitude(pressaoAtual, PRESSAO_MAR);
    alturaMinima = alturaInicial;

    // inicializar o cartão SD
    if (!SD.begin(PINO_SD_CS))
    {
        erro = ERRO_SD;
        return;
    }
    else if (!erro)
    {
        int n = 1;
        bool parar = false;

        while (!parar)
        {
#ifdef DEBUG
            Serial.println("Contando arquivos");
#endif
            sprintf(nomeConcat, "/log%d.txt", n);
            if (SD.exists(nomeConcat))
                n++;
            else
                parar = true;
        }

        arquivoLog = SD.open(nomeConcat, FILE_WRITE);
        arquivoLog.close();

#ifdef DEBUG
        Serial.print("Salvando o arquivo ");
        Serial.println(nomeConcat);
#endif
    }

    if (!erro)
    {
#ifdef DEBUG
        Serial.println("Nenhum erro iniciando dispositivos");
#endif
        statusAtual = ESTADO_ESPERA;
    }

    else
    {
#ifdef DEBUG
        Serial.print("Altímetro com erro de inicialização do código:");
        Serial.println(erro);
#endif
        statusAtual = erro;
    }
}

void notificaCodigo(void *pvParameters)
{
    while (1)
    {

#ifdef DEBUG
        Serial.print("Status atual do altímetro:");
        Serial.println(statusAtual);
#endif
        switch (statusAtual)
        {
        // Problema com o BMP, LED AZUL
        case ERRO_BMP:
            digitalWrite(PINO_BUZZER, !digitalRead(PINO_BUZZER));
            digitalWrite(PINO_LED_AZUL, !digitalRead(PINO_LED_AZUL));
            digitalWrite(PINO_LED_VERD, LOW);
            digitalWrite(PINO_LED_VERM, LOW);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            break;

        // Problema com o SD, LED VERMELHO
        case ERRO_SD:
            digitalWrite(PINO_BUZZER, !digitalRead(PINO_BUZZER));
            digitalWrite(PINO_LED_VERM, !digitalRead(PINO_LED_VERM));
            digitalWrite(PINO_LED_AZUL, LOW);
            digitalWrite(PINO_LED_VERD, LOW);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            break;

        // Estado onde o voo já terminou
        case ESTADO_RECUPERANDO:
            digitalWrite(PINO_BUZZER, !digitalRead(PINO_BUZZER));
            digitalWrite(PINO_LED_VERD, !digitalRead(PINO_LED_VERD));
            digitalWrite(PINO_LED_AZUL, LOW);
            digitalWrite(PINO_LED_VERM, LOW);
            vTaskDelay(250 / portTICK_PERIOD_MS);
            break;

        // Gravando
        case ESTADO_GRAVANDO:
            digitalWrite(PINO_BUZZER, !digitalRead(PINO_BUZZER));
            digitalWrite(PINO_LED_AZUL, !digitalRead(PINO_LED_AZUL));
            digitalWrite(PINO_LED_VERD, LOW);
            digitalWrite(PINO_LED_VERM, LOW);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            break;
        // Espera
        case ESTADO_ESPERA:
            digitalWrite(PINO_LED_VERD, !digitalRead(PINO_LED_VERD));
            digitalWrite(PINO_LED_AZUL, LOW);
            digitalWrite(PINO_LED_VERM, LOW);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            break;
        default:
            vTaskDelay(100 / portTICK_PERIOD_MS);
            break;
        }
    }
}
void adquireDadosCodigo(void *pvParameters)
{
    while (1)
    {
        bmp.getTemperatureAndPressure(temperaturaAtual, pressaoAtual);
        alturaAtual = bmp.altitude(pressaoAtual, PRESSAO_MAR);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
void gravaDadosCodigo(void *pvParameters)
{
    while (1)
    {
        // verifica aqui o estado do foguete e também se o arquivo está aberto e pronto
        // para ser usado. Aqui, todos os dados são concatenados em uma string que dá
        // o formato das linhas do arquivo de log.
        if ((statusAtual == ESTADO_GRAVANDO) || (statusAtual == ESTADO_RECUPERANDO))
        {
            arquivoLog = SD.open(nomeConcat, FILE_APPEND);
#ifdef DEBUG
            Serial.println("Estou gravando!");
            Serial.println(nomeConcat);
#endif
            String stringDados = "";
            unsigned long millisGravacao = millis();
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
#ifdef DEBUG
            Serial.println(stringDados);
#endif

            if(arquivoLog.print(stringDados))
                Serial.println("Gravei dados no SD!");
            arquivoLog.close();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
void recuperaCodigo(void *pvParameters)
{
    while (1)
    {
        // Funcao responsavel por checar condicoes e atualizar variaveis de extremos
        // (altura maxima, altura minima, etc)

        if (statusAtual == ESTADO_GRAVANDO)
        {
            alturaMinima = alturaAtual;
        }
        // alturaMinima
        if ((alturaAtual < alturaMinima) && (statusAtual == ESTADO_GRAVANDO))
            alturaMinima = alturaAtual;

        // alturaMaxima
        if (!subiu && (statusAtual == ESTADO_GRAVANDO))
            alturaMaxima = 0;

        // controle de subida
        if ((alturaAtual > alturaMinima + THRESHOLD_SUBIDA) && (statusAtual == ESTADO_GRAVANDO) && !subiu)
            subiu = true;

        // primeira referencia de altura maxima
        if (subiu && (alturaMaxima == 0) && (statusAtual == ESTADO_GRAVANDO))
            alturaMaxima = alturaAtual;

        // verificar a altura máxima
        if ((alturaAtual > alturaMaxima) && (statusAtual == ESTADO_GRAVANDO) && subiu)
            alturaMaxima = alturaAtual;

        // Controle de descida, usando um threshold para evitar disparos não
        // intencionais
        if ((alturaAtual + THRESHOLD_DESCIDA < alturaMaxima) && (statusAtual == ESTADO_GRAVANDO) && subiu)
        {
            descendo = true;
            subiu = false;
            statusAtual = ESTADO_RECUPERANDO;
            if (descendo && !abriuParaquedas)
            {
#ifdef DEBUG
                Serial.println("Abrindo o paraquedas!");
#endif
                digitalWrite(REC_PRINCIPAL, HIGH);
                millisRec = millis(); // armazena o horário que o paraquedas foi aberto
                abriuParaquedas = 1;
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
void setup()
{
#ifdef DEBUG
    Serial.begin(115200);
    Serial.println("Iniciando o altímetro");
#endif
    inicializa();
    xTaskCreate(notificaCodigo, "notifica", 2048, NULL, 1, &notifica);
    xTaskCreate(adquireDadosCodigo, "adquireDados", 2048, NULL, 1, &adquireDados);
    xTaskCreate(gravaDadosCodigo, "gravaDados", 2048, NULL, 1, &gravaDados);
    xTaskCreate(recuperaCodigo, "recupera", 2048, NULL, 1, &recupera);
}

void loop()
{
    delay(10);
}