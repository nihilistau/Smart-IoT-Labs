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
  uint8_t frame[16]; // trames avec octets
  float  data[4];    // 4 valeurs en virgule flottante
} pack_t ;  // paquet d’émission

QueueHandle_t dqueue;  // queues for data packets

void onReceive(int packetSize) 
{ 
pack_t rdp;  int i=0; 
if (packetSize == 0) return;   // if there's no packet, return 
if (packetSize==16) 
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
dqueue = xQueueCreate(4,16); // queue for 4 data packets
LoRa.onReceive(onReceive);  // pour indiquer la fonction ISR
LoRa.receive();             // pour activer l'interruption (une fois)
}

uint32_t mindel=10000;  // 10 seconds


void loop()
{
pack_t rp;    // packet elements to send
  xQueueReceive(dqueue,rp.frame,portMAX_DELAY); // 6s,default:portMAX_DELAY 
  ThingSpeak.setField(1,rp.data[0]); 
  ThingSpeak.setField(2,rp.data[1]); 
  ThingSpeak.setField(3,rp.data[2]); 
  ThingSpeak.setField(4,rp.data[3]); 
  Serial.printf("d1=%2.2f,d2=%2.2f,d3=%2.2f,d4=%2.2f\n",rp.data[0],rp.data[1],rp.data[2],rp.data[3]);
  while (WiFi.status() != WL_CONNECTED) {  delay(500);  } 
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  
  delay(mindel);  // mindel is the minimum waiting time before sending to ThingSpeak         
 //LoRa.receive(); 
} 
