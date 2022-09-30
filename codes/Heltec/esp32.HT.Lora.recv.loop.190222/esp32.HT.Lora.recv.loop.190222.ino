
#include <SPI.h>              // include libraries
#include <LoRa.h>
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define freq    868E6   
#define sf 8
#define sb 125E3  


union
{
 uint8_t frame[8]; // trames avec octets
  float  sensor[2];   // 2 valeurs en virgule flottante
} pack;

void onReceive(int packetSize) 
{ 
int i=0,rssi=0; 
  if (packetSize == 0) return;   // if there's no packet, return 
  i=0; 
  if (packetSize==8) 
    { 
    while (LoRa.available()) { 
      pack.frame[i]=LoRa.read();i++; 
      } 
      rssi=LoRa.packetRssi(); 
    } 
}

void setup() {
  Serial.begin(9600);
  delay(1000); 
  pinMode(DI0,INPUT); 
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa init OK");
LoRa.setSpreadingFactor(sf);
LoRa.setSignalBandwidth (sb);
LoRa.onReceive(onReceive);  // pour indiquer la fonction ISR
LoRa.receive();  // pour activer l'interruption (une fois)
  delay(1000);
}

float d1,d2;

void loop()  // la boucle de l’emetteur
{
  d1=pack.sensor[0];d2=pack.sensor[1];
  Serial.println(d1);  Serial.println(d2);
  delay(3000);
  LoRa.receive();  // pour activer l'interruption (une fois)
}
