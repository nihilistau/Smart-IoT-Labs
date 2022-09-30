#include <SPI.h>               
#include <LoRa.h>
#define SCK     18   // GPIO18 -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    23   // GPIO23 -- SX127x's MOSI
#define SS       5   // GPIO05 -- SX127x's CS
#define RST     15   // GPIO15 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define freq    434E6   
#define sf 7
#define sb 125E3      


void onReceive(int packetSize) 
{ 
int rssi=0; 
union pack
  {
  uint8_t frame[16]; // trames avec octets
  float  data[4];    // 4 valeurs en virgule flottante
  } rdp ;  // paquet de réception
  if (packetSize == 0) return;   // if there's no packet, return 
  int i=0; 
  if (packetSize==16) 
    { 
    while (LoRa.available()) 
      { 
      rdp.frame[i]=LoRa.read();i++; 
      } 
      rssi=LoRa.packetRssi(); 
      Serial.printf("Received packet:%2.2f,%2.2f\n",rdp.data[0],rdp.data[1]);
      Serial.printf("RSSI=%d\n",rssi);
    } 
}

void setup() {
  Serial.begin(9600);                  
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
 
