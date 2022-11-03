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

tspack_t spack;   // sending packet

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
  spack.data.head[0]=0xFF;spack.data.head[1]=0x01;
  spack.data.head[2]=0x00;spack.data.head[3]=0x00;
  memcpy(spack.data.key,wkey,16);
  spack.data.chnum=channel;
}



float s1=0.1, s2=0.2;

void loop()  // la boucle de l’emetteur
{
Serial.println("New Packet:") ;
  LoRa.beginPacket();                   // start packet
  spack.data.sensor[0]=s1;spack.data.sensor[1]=s2;
  LoRa.write(spack.buff,40);
  LoRa.endPacket();
  Serial.print(s1);Serial.print("  ");Serial.println(s2);
  s1=s1+0.1; s2=s2+0.2;
  delay(6000);
}

     
