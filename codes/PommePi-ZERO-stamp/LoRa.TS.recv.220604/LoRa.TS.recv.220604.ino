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

#define wkey "3IN09682SQX3PT4Z"
#define channel 1626377

typedef union
{
uint8_t  buff[40];   // 40 or 72 bytes
struct 
  {
  uint8_t head[4];  // packet header: address, control, ..
  char key[16];  // read or write APi key
  int chnum;     // channel number
  float sensor[4];  // or sensor[8]
  } data;
} tspack_t;

tspack_t rpack;   // sending packet

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


float s1=0.1, s2=0.2;
int rssi;

void loop()  // la boucle de l’emetteur
{
int packetLen;
packetLen=LoRa.parsePacket();
if(packetLen==40)
  {
  int i=0;
  Serial.println("packet recv");
  while (LoRa.available()) {
    rpack.buff[i]=LoRa.read();i++;
    }
  if(rpack.data.head[0]==0x01)
    {
    s1=rpack.data.sensor[0]; s2=rpack.data.sensor[1];
    rssi=LoRa.packetRssi();  // force du signal en réception en dB 
    Serial.printf("T=%2.2f,H=%2.2f\n",s1,s2);
    Serial.printf("RSSI=%d\n",rssi);
    }
  }  
}

     
