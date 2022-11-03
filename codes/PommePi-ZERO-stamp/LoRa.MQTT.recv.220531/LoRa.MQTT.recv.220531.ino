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

typedef union
{
uint8_t  buff[64];   // total data –64 bytes
struct 
  {
  char topic[32];  // MQTT topic
  char mess[32];   // MQTT message
  } data;
} mqttpack_t;

mqttpack_t rpack;

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

char topic[32],mess[32];
int rssi;

void loop()   
{
int packetLen;
packetLen=LoRa.parsePacket();
if(packetLen==64)
  {
  int i=0;
  while (LoRa.available()) {
    rpack.buff[i]=LoRa.read();i++;
    }
  strcpy(topic,rpack.data.topic);strcpy(mess,rpack.data.mess);
  rssi=LoRa.packetRssi();  // force du signal en réception en dB 
  Serial.printf("Topic=%s,Message=%s\n",topic,mess);
  Serial.printf("RSSI=%d\n",rssi);
  }
}
 
