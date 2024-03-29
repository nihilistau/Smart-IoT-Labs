#include <SPI.h>               
#include <LoRa.h>
#define SCK     6   // GPIO18 -- SX127x's SCK
#define MISO    0 //7   // GPIO19 -- SX127x's MISO
#define MOSI    1 //8   // GPIO23 -- SX127x's MOSI
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
} rdp ;  // paquet de réception

float d1=0.0, d2=0.0 ;
int rssi;

void onReceive(int packetLen) {

  // received a packet
  if(packetLen==16)
  {
  for (int i = 0; i < 16; i++){
    rdp.frame[i]=LoRa.read();
    }
  Serial.printf("RSSI=%d\n",LoRa.packetRssi());
  }
  LoRa.receive();
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
LoRa.onReceive(onReceive);
// put the radio into receive mode
LoRa.receive();
}


void loop()   
{
int i;
Serial.printf("loop():Received packet:%2.2f,%2.2f\n",rdp.data[0],rdp.data[1]);
delay(2000);
}
 
