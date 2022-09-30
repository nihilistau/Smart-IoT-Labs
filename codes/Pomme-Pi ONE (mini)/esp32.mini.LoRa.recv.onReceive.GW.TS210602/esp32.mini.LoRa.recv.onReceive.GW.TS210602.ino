#include <SPI.h>               
#include <LoRa.h>
#include <WiFiManager.h> 
#include "ThingSpeak.h"
unsigned long myChannelNumber = 1;   
const char *myWriteAPIKey="HEU64K3PGNWG36C4" ;
const char *myReadAPIKey="AVG5MID7I5SPHIK8";
WiFiClient  client;

#define SCK     18   // GPIO18 -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    23   // GPIO23 -- SX127x's MOSI
#define SS       5   // GPIO05 -- SX127x's CS
#define RST     15   // GPIO15 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define freq    434E6   
#define sf 7
#define sb 125E3  

typedef union 
  { 
  uint8_t frame[40];   
  struct  
    { 
    uint8_t head[4];   // packet header
    int chnum;         // channel number
    char key[16];      // write or read key
    float sensor[4]; 
    } pack; 
  } pack_t;


void onReceive(int packetSize) 
{ 
int rssi=0; 
pack_t rdp;

  if (packetSize == 0) return;   // if there's no packet, return 
  int i=0; 
  if (packetSize==40) 
    { 
    while (LoRa.available()) 
      { 
      rdp.frame[i]=LoRa.read();i++; 
      } 
      rssi=LoRa.packetRssi(); 
      Serial.printf("Received packet:%2.2f,%2.2f\n",rdp.pack.sensor[0],rdp.pack.sensor[1]);
      Serial.printf("RSSI=%d\n",rssi);
    } 
}

void setup() {
  Serial.begin(9600);  
  WiFi.mode(WIFI_STA); 
  WiFiManager wm;
  // wm.resetSettings();
  bool res;
  res = wm.autoConnect("ESP32AP",NULL); // password protected ap
  if(!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }   
  ThingSpeak.begin(client); // connexion (TCP) du client au serveur
  delay(1000);
  Serial.println("ThingSpeak begin"); 
  Serial.println("Start Lora");               
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  Serial.println();delay(100);Serial.println();
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
Serial.println("Starting LoRa OK!");
LoRa.setSpreadingFactor(sf);
LoRa.setSignalBandwidth(sb);
LoRa.onReceive(onReceive);  // pour indiquer la fonction ISR
LoRa.receive();             // pour activer l'interruption (une fois)
}


void loop()   
{
Serial.println("in the loop");
delay(5000);
}
 
