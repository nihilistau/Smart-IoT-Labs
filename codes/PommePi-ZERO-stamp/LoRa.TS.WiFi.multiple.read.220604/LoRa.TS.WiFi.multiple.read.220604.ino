#include <WiFi.h>
#include "ThingSpeak.h"  
#include <Wire.h>
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
//const char *ssid     = "PhoneAP";
//const char *pass = "smartcomputerlab";
const char* ssid = "Livebox-08B0";
const char* pass = "G79ji6dtEptVTPWmZP";
WiFiClient  client;
unsigned long myChannelNumber = 1626377;
const char * myReadAPIKey = "9JVTP8ZHVTB9G4TT";
int number = 0;
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

void setup_LoRa() 
{                  
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

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network251
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void setup() {
  Serial.begin(9600);
  setup_LoRa(); delay(400);
  setup_wifi();
}

int field[8] = {1,2,3,4,5,6,7,8};
// Initialize our values
float s1,s2,s3,s4;

void loop() {    
   // Connect or reconnect to WiFi
    while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");   delay(1000);     
    } 
    Serial.println("\nConnected.");
    int statusCode = ThingSpeak.readMultipleFields(myChannelNumber,myReadAPIKey);    
    if(statusCode == 200)
    {
     s1 = ThingSpeak.getFieldAsFloat(field[0]);  
     s2 = ThingSpeak.getFieldAsFloat(field[1]);  
     s3 = ThingSpeak.getFieldAsFloat(field[2]);  
     s4 = ThingSpeak.getFieldAsFloat(field[3]);  
     String createdAt = ThingSpeak.getCreatedAt(); // Created-at timestamp
     Serial.println(s1);Serial.println(s2);
     Serial.println(createdAt);
     LoRa.beginPacket(); // start packet
     spack.data.head[0]=0x01;     // first client terminal
     spack.data.sensor[0]=s1;spack.data.sensor[1]=s2;
     LoRa.write(spack.buff,40);
     LoRa.endPacket();
     
    }
    else{
      Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
    }
    Serial.println();
    delay(20000);    
}
