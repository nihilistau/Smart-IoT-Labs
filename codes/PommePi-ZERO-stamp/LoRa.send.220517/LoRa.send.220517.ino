#include <SPI.h>               
#include <LoRa.h>
#define SCK     6   // GPIO18 -- SX127x's SCK
#define MISO    0   // GPIO19 -- SX127x's MISO
#define MOSI    1   // GPIO23 -- SX127x's MOSI
#define SS      10   // GPIO05 -- SX127x's CS
#define RST     4   // GPIO15 -- SX127x's RESET
#define DI0     5   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define freq    434E6   
#define sf 7
#define sb 125E3      

union pack
{
  uint8_t frame[16]; // trames avec octets
  float  data[4];    // 4 valeurs en virgule flottante
} sdp ;  // paquet d’émission

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
}

float d1=0.0, d2=0.0 ;

void loop()  // la boucle de l’emetteur
{
Serial.print("New Packet:") ;
  LoRa.beginPacket();                   // start packet
  sdp.data[0]=d1;
  sdp.data[1]=d2;
  LoRa.write(sdp.frame,16);
  LoRa.endPacket();
  Serial.printf("%2.2f,%2.2f\n",d1,d2);
  d1=d1+0.1; d2=d2+0.2;
  delay(2000);
}
 
