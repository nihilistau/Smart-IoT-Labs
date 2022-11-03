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
// Replace the next variables with your SSID/Password combination
//const char *ssid     = "PhoneAP";
//const char *pass = "smartcomputerlab";
const char* ssid = "Livebox-08B0";
const char* pass = "G79ji6dtEptVTPWmZP";


WiFiClient  client;


char wkey[17];
int channel;

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

void setup() {
  Serial.begin(9600);
  setup_LoRa(); delay(400);
  setup_wifi();
}

int rssi;

void loop() 
{
int packetLen;
packetLen=LoRa.parsePacket();
if(packetLen==40)
  {
  int i=0;
  while (LoRa.available()) 
    {
    rpack.buff[i]=LoRa.read();i++;
    }
  strncpy(wkey,rpack.data.key,16);wkey[16]=0x00;channel=rpack.data.chnum;
  Serial.println(wkey);Serial.println(channel);
  rssi=LoRa.packetRssi();  // force du signal en réception en dB 
  ThingSpeak.setField(1, rpack.data.sensor[0]);
  ThingSpeak.setField(2, rpack.data.sensor[1]);
  ThingSpeak.setField(3, rpack.data.sensor[2]);
  ThingSpeak.setField(4, rpack.data.sensor[3]);
// write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(channel, wkey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }  
  Serial.println("Packet sent to TS");
  delay(15000);
  }
}
