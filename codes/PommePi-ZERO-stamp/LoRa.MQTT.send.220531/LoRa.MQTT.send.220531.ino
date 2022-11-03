#include <SPI.h>               
#include <LoRa.h>
// connection lines – SPI bus
#define SCK     6   // GPIO18 -- SX127x's SCK
#define MISO    0   // GPIO19 -- SX127x's MISO
#define MOSI    1   // GPIO23 -- SX127x's MOSI
#define SS     10   // GPIO05 -- SX127x's CS
#define RST     4   // GPIO15 -- SX127x's RESET
#define DI0     5   // GPIO26 -- SX127x's IRQ(Interrupt Request)
// radio parameters
#define freq    434E6   
#define sf 7
#define sb 125E3 

typedef union
{
uint8_t  buff[64];   // total data –64 bytes
struct 
  {
  char topic[32];  // MQTT topic
  char mess[32];   // MQTT message
  } data;
} mqttpack_t;

mqttpack_t spack;

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
strcpy(spack.data.topic,"risc-v/test");
}



float d1=0.1, d2=0.2;

void loop()  // la boucle de l’emetteur
{
Serial.print("New Packet:") ;
  LoRa.beginPacket();                   // start packet
  sprintf(spack.data.mess,"T:%2.2f,H:%2.2f",d1,d2);
  LoRa.write(spack.buff,64);
    LoRa.endPacket();
  Serial.println(spack.data.mess);
  d1=d1+0.1; d2=d2+0.2;
  delay(6000);
}

     
