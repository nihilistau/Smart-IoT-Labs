#include <SPI.h>               
#include <LoRa.h>
#include <WiFiManager.h> 
#include "ThingSpeak.h"
unsigned long myChannelNumber = 1;   
const char *myWriteAPIKey="HEU64K3PGNWG36C4";
const char *myReadAPIKey="AVG5MID7I5SPHIK8";
WiFiClient  client;

#define SCK     18   // GPIO18 -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    23   // GPIO23 -- SX127x's MOSI
#define SS       5   // GPIO05 -- SX127x's CS
#define RST     15   // GPIO15 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define freq    434E6   
#define sf      7
#define sb      125E3  

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

QueueHandle_t dqueue;  // queues for data packets

void onReceive(int packetSize) 
{ 
pack_t rdp;  int i=0; 
if (packetSize == 0) return;   // if there's no packet, return 
if (packetSize==40) 
  { 
  while (LoRa.available())  {  rdp.frame[i]=LoRa.read();i++; } 
  xQueueReset(dqueue); // to keep only the last element 
  xQueueSend(dqueue, &rdp, portMAX_DELAY); 
  } 
}

void setup() 
{
  Serial.begin(9600);  
  WiFi.mode(WIFI_STA); 
  WiFiManager wm;
  // wm.resetSettings();
  bool res;
  res = wm.autoConnect("ESP32AP",NULL); // password protected ap
  if(!res) 
    {
    Serial.println("Failed to connect");// ESP.restart();
    } 
  else 
    {
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
dqueue = xQueueCreate(4,40); // queue for 4 data packets
LoRa.onReceive(onReceive);  // pour indiquer la fonction ISR
LoRa.receive();             // pour activer l'interruption (une fois)
}


void loop()
{
pack_t rp;    // packet elements to send
xQueueReceive(dqueue,rp.frame,portMAX_DELAY); // 6s,default:portMAX_DELAY 
if(rp.pack.head[1]==0x01 || rp.pack.head[1]==0x00) 
  { 
  if(rp.pack.head[2]&0x80) ThingSpeak.setField(1,rp.pack.sensor[0]); 
  if(rp.pack.head[2]&0x40) ThingSpeak.setField(2,rp.pack.sensor[1]); 
  if(rp.pack.head[2]&0x20) ThingSpeak.setField(3,rp.pack.sensor[2]); 
  if(rp.pack.head[2]&0x10) ThingSpeak.setField(4,rp.pack.sensor[3]); 
  while (WiFi.status() != WL_CONNECTED) {  delay(500);  } 
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); 
  }           
 //LoRa.receive(); 
} 
