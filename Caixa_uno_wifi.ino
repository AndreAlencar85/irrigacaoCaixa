// ---------------------------------------------------------------------------
// Example NewPing library sketch that does a ping about 20 times per second.
// compilacao: processador atmega 168 / porta: usbseral 1420
// v0: Arduino Uno na Cisterna enviando informações para Mega
// v1: Rede com 3 arduinos: this: node 01 (Cisterna)
// v0: uno fica na caixa
 /* Antes:                   Mega: Node 00
 *
 *   Uno (cisterna): Node 01         Nano (Caixa): Node 02 (this.node)
 *   
 * Agora:                   Mega: Node 00
 *
 *   Uno (caixa): Node 01         Nano (Cisterna): Node 02 (this.node)
*/
// ---------------------------------------------------------------------------


// #include <printf.h>
// #include <nRF24L01.h>
// #include <RF24_config.h>
#include <RF24.h>
#include <RF24Network.h>
#include <SPI.h>

#include <NewPing.h>
#include <LiquidCrystal.h>

#define MAX_DISTANCE 400 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define NUMREADINGS 10 // tamanho da janela para cálculo da média móvel
#define TRIGGER_PIN_CIS  9  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN_CIS     8  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define H_CIS 90         // Altura (cm) da cisterna  caixa = 90  cis = 155
#define h_SENSOR_CIS 22         // Distância (cm) entre o sensor e o inicio da cisterna  cis=25 caixa = 22

#define tempo_TX 5000
#define tempo 1000  

// --- Declaraçao de Variáveis --- //
int readings_cis[NUMREADINGS];
int index_cis = 0;                            // índice da leitura atual
int total_cis = 0;                            // total móvel
int average_cis = 0;                          // média
int leitura_cis=0;
int nivel_cis=0;
float nivel_cis_percent=0;
long tempoInicio;
boolean falhaTX;

struct estruturaDadosRF
{
   int nivel = 0;
   int nivel_percent = 0;
};
typedef struct estruturaDadosRF tipoDadosRF;

tipoDadosRF dadosRF;


// --- Declaracao de Objetos --- //
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);    // Indica as portas digitais utilizadas: lcd(<pino RS>, <pino enable>, <pino D4>, <pino D5>, <pino D6>, <pino D7>)
NewPing sonar_cis(TRIGGER_PIN_CIS, ECHO_PIN_CIS, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

RF24 radio(14,15);   // CE,CS = CHIP ENABLE E CHIP SELECT
RF24Network network(radio);      // Include the radio in the network
const uint16_t node00 = 00; 
const uint16_t this_node = 01;   // Address of our node in Octal format ( 04,031, etc)

//byte enderecos[][6] = {"1node","2node"};



void setup() {
  Serial.begin(9600); // Open serial monitor at 115200 baud to see ping results.
  
    // Config LCD
  lcd.begin(16, 2); // 16 colunas e 2 linhas
  lcd.clear();
  lcd.print("Setup");

    // config comunicacao wifi 
  SPI.begin();
  radio.begin();
  network.begin(90, this_node); //(channel, node address)
    
    /*
  radio.begin();
  radio.openWritingPipe(enderecos[1]);
  radio.openReadingPipe(1,enderecos[0]);
  radio.stopListening(); 
  //radio.startListening(); */
   
  
  // Inicializa vetor de leituras
  for (int i = 0; i < NUMREADINGS; i++) 
  {
      readings_cis[i] = 0;                      // inicializa todas as leituras com 0
    }
    
}




void loop()
{
  
    Serial.println("loop"); 

    leitura_cis=sonar_cis.ping_cm();
    
    if ( leitura_cis!=0) 
    {
        Serial.println(leitura_cis);
        total_cis -= readings_cis[index_cis];               // subtrair a última leitura
        readings_cis[index_cis] = leitura_cis;              // ler do sensor
        total_cis += readings_cis[index_cis];               // adicionar leitura ao total
        index_cis = (index_cis + 1);                        // avançar ao próximo índice
  
        if (index_cis >= NUMREADINGS)                       // se estiver no fim do vetor...
        {                
            index_cis = 0;                                  // ...meia-volta ao início
            //delay(tempo);
        }
    
    average_cis = total_cis / NUMREADINGS;                  // calcular a média
    nivel_cis = H_CIS + h_SENSOR_CIS - average_cis ;   
    nivel_cis_percent = (nivel_cis * 100) / H_CIS; 
    
    dadosRF.nivel = nivel_cis;
    //dadosRF.nivel = leitura_cis;
    dadosRF.nivel_percent = int(nivel_cis_percent);

    
    tempoInicio = millis();
    network.update();
    Serial.println("update");
    
    RF24NetworkHeader header(/*to node*/node00);     // (Address where the data is going)
    
    //while ( falhaTX = !radio.write(&dadosRF,sizeof(tipoDadosRF))) 
    while ( falhaTX = !network.write(header,&dadosRF,sizeof(tipoDadosRF))) 
    { 
        Serial.print("Tentando por: ");
        Serial.print(tempo_TX/1000);
        Serial.println(" segundos");
        
        if ((millis() - tempoInicio) > tempo_TX) 
        {
            break;
            
         }  
    }


    if (falhaTX == 1) 
    {
        lcd.clear();
        lcd.setCursor(5,1);
        lcd.print("Falha TX...");
        Serial.println("Falha TX..."); 
    }
    else 
    {
        lcd.clear();
        lcd.setCursor(5,1);
        lcd.print("Sucesso TX"); 
        Serial.println("Sucesso TX");
        //delay(tempo);
    }


    
    lcd.setCursor(0, 0);
    //lcd.print("Cist:");
    lcd.print("L:");
    lcd.print(leitura_cis);
    lcd.print(" N:");
    lcd.print(nivel_cis);
    //lcd.print("cm");
    lcd.setCursor(0, 1);
    //lcd.print(" ");
    lcd.print(int(nivel_cis_percent));
    lcd.print("% ");

    }

}
