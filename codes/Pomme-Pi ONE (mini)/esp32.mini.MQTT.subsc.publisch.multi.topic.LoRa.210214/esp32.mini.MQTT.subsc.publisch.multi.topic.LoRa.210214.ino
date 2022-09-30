#include <WiFi.h>
#include <MQTT.h>
#include <Wire.h> 
#include <LoRa.h>
 

#define NT  4  // max number of terminals
#define NTS 4  // max number of sensors per terminal

const char* ssid = "Livebox-08B0";
const char* pass = "G79ji6dtEptVTPWmZP";


const char* mqttServer = "broker.emqx.io";


WiFiClient net;
MQTTClient client;
unsigned long lastMillis = 0;

#define SS       5 // 26     // D0 - to NSS
#define RST     15  //16     // D4  - RST
#define DI0     26      // D8 - INTR

#define SCK     18      // D5 - CLK
#define MISO    19      // D6 - MISO
#define MOSI    23      // D7 - MOSI
#define BAND    434E6

int sf=7;
long sbw=125E3;

union 
{
  uint8_t buff[64];
  char mess[64];
  struct
    {
      uint8_t head[4];
      float sens[7];
    } pack;
} sdf,rdf;  // sent data frame, received data frame

int R_LoRa=0;  // receive indicator
int S_LoRa=0;  // send indicator

void LoRa_recv()
{
int packetSize,i=0;
  // try to parse packet
while(1)
  {
  packetSize = LoRa.parsePacket();
  if (packetSize) 
    {
    for(i=0;i<packetSize;i++)rdf.buff[i]=LoRa.read(); R_LoRa=1;
    }
  }
}  

void LoRa_send(char *topic, char *mess)  // here you put sdf.mess  to send
{
  // send packet
  // topic to header  /esp32_GW1/%d/%d  - rdf.pack.head[2],rdf.pack.head[3]
  // topic+12 - first digit (ASCI) / 
  sdf.pack.head[
  LoRa.beginPacket();
  LoRa.write(sdf.buff,64);
  LoRa.endPacket();
}
    

void connect() {
  char cbuff[128];
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nconnecting...");
  while (!client.connect("IoT.GW1")) {
    Serial.print("."); delay(1000);
  }
  Serial.println("\nIoT.GW1 - connected!");
  for(int i=0; i<NT ; i++)
    for(int j=0; j<NTS; j++)
      {
      sprintf(cbuff,"/esp32_GW1/Term%d/Sens%d",i,j); 
      Serial.println(cbuff); 
      client.subscribe(cbuff);
      delay(1000);
      }
  
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  // send LoRa message depending on topic
  
  LoRa_send(
}


void setup() {
  Serial.begin(9600);
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);
 if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  else 
   {
   Serial.println("Starting LoRa ok!");
   LoRa.setSpreadingFactor(sf);
   LoRa.setSignalBandwidth(sbw);
   }
   
  WiFi.begin(ssid, pass);
  client.begin(mqttServer, net);
  client.onMessage(messageReceived);
  connect();
}

void loop() {
  int tn,tsn, val; char cbuff[128], vbuff[15];
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability
  if (!client.connected()) { connect(); }
  if (R_LoRa) 
    {   // publish a message every 20 seconds
    sprintf(cbuff,"/esp32_GW1/%d/%d",rdf.pack.head[2],rdf.pack.head[3]);
    sprintf(vbuff,"%f",rdf.pack.sens[0]);  // may be rdf.pack.sens[rdf.pack.head[3]] 
    client.publish(cbuff, vbuff); R_LoRa=0;
    delay(4000); // or here R_LoRa=0;

  }
}
