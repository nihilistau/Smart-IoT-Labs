#include <WiFi.h>
#include <MQTT.h> 
#include "LoRa_Frames.h"
#include <SPI.h>               
#include <LoRa.h>
#include <Wire.h>                
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL

const char* ssid = "Livebox-08B0";
const char* pass = "G79ji6dtEptVTPWmZP";

const char* mqttServer = "broker.emqx.io";
WiFiClient net;
MQTTClient client;


QueueHandle_t dqueue, mqttqueue;
int queueSize = 32;
static int taskCore = 0;

uint64_t chipid;  
uint32_t gwID;
uint32_t recvID;

#define SS       5 // 26     // D0 - to NSS
#define RST     15  //16     // D4  - RST
#define DI0     26      // D8 - INTR

#define SCK     18      // D5 - CLK
#define MISO    19      // D6 - MISO
#define MOSI    23      // D7 - MOSI
#define BAND    868E6

int sf=7;
long sbw=125E3;

uint16_t timeout;
char *gtpass="passwordpassword";

conframe_t scf,rcf;  // defined in LoRa_Frames.h
MQTTframe_t sdf,rdf;
int ack_mess=0;
RTC_DATA_ATTR int ack_idreq=0, ack_dtsub=0;

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  topic.toCharArray(sdf.pack.topic,24);
  payload.toCharArray(sdf.pack.mess,24);
  ack_mess=1;
}


void MQTT_Task( void * pvParameters ){  // subscribe task
float stab[8]; char kbuff[32];
MQTTframe_t rdf;
while(true)
  {
  xQueueReceive(mqttqueue, &rdf.pack, portMAX_DELAY);
  LoRa.idle(); 
  Serial.printf("%s\n",rdf.pack.topic);
  if (!client.connected()) { connect(); }
  client.subscribe(rdf.pack.topic);
  Serial.println("topic subscribed");
  delay(10000);
  LoRa_rxMode(); 
  }
}

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnecting...");
  while (!client.connect("IoT.GW4")) {
    Serial.print(".");
    delay(1000);}
  Serial.printf("IoT.GW4 - connected!\n");
}





void setup() {
Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  gwID=(uint32_t)chipid;

  WiFi.begin(ssid, pass);
    Serial.print("\nconnecting...");

  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  client.begin(mqttServer, net);
  client.onMessage(messageReceived);
  Serial.println("\nconnected!");
  if (!client.connected()) { connect(); }
  Serial.println();Serial.println();
  Serial.println("WiFi connected");
  
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);

  Serial.begin(9600);
  delay(1000);
  Serial.println();Serial.println();
  Serial.println("Starting Gateway - mode 3");
  Serial.printf("%08X\n",(uint32_t)chipid);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Starting LoRa ok!");
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sbw);
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Simple Gateway");
  Serial.println("Only receive messages from nodes");
  Serial.println("Tx: invertIQ enable");
  Serial.println("Rx: invertIQ disable");
  Serial.println();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
  mqttqueue = xQueueCreate(queueSize, sizeof(rdf));
  xTaskCreatePinnedToCore(
                    MQTT_Task,   /* Function to implement the task */
                    "MQTT_Task", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore);  /* Core where the task should run */
  Serial.println("Task created...");
}

void loop() {
  if (ack_idreq) 
    { 
      delay(200);send_ACK(0x42); ack_idreq=0;Serial.println("IDREQ ACK sent");  // MQTT SUB - mode 4
    }
    
  if (ack_dtsub) 
    { 
      delay(200);send_ACK(0x46); ack_dtsub=0;Serial.println("DTSUB ACK sent");
    }

   if(ack_mess)
   {
    delay(200);send_DT(0x48); ack_mess=0; 
   }

delay(100);  
}

void LoRa_rxMode(){
  LoRa.disableInvertIQ();               // normal mode
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.enableInvertIQ();                // active invert I and Q signals
}

void send_ACK(uint8_t con)             // ACK frame with specific control field
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  if(con==0x42) scf.pack.did=(uint32_t)rcf.pack.sid;   // ack for IDREQ frame - mode 3
  if(con==0x46) scf.pack.did=(uint32_t)rdf.pack.sid;   // ack for DTSUB frame 
  scf.pack.sid=(uint32_t)gwID;  // send ACK frame with the received nodeID
  scf.pack.con[0]=con;scf.pack.con[1]=0x00;  // con[0] depends on the type af ACK frame: IDREQ,DTACK,..
  strncpy(scf.pack.pass,"passwordpassword",16);
  scf.pack.tout=20;                     // timeout for the next data frame
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
  Serial.printf("ACK con=%0X,timeout=%d, dest=%08X\n",con,scf.pack.tout,scf.pack.did);
}

void send_DT(uint8_t con)
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  sdf.pack.did=(uint32_t)rdf.pack.sid;   // ack for IDREQ frame - mode 3
  sdf.pack.sid=(uint32_t)gwID;  // send ACK frame with the received nodeID
  sdf.pack.con[0]=con;scf.pack.con[1]=0x00;  // con[0] depends on the type af ACK frame: IDREQ,DTACK,..
  // sdf.pack.topic  and sdf.pack.mess set in messageReceived()
  sdf.pack.tout=20;                     // timeout for the next data frame
  LoRa.write(sdf.frame,64);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
  Serial.printf("SUB DTSND con=%0X,timeout=%d, dest=%08X\n",con,sdf.pack.tout,sdf.pack.did);
}

void onReceive(int packetSize) 
{
  if(packetSize==32)
    { 
    int i=0;
    while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(rcf.pack.con[0]==0x41 && rcf.pack.did==(uint32_t)0 && !strncmp(rcf.pack.pass,gtpass,16))  
    // received IDREQ frame for MQTT subscribe server
      {
      Serial.print("Received IDREQ frame from ");Serial.printf("%08X\n",(uint32_t)rcf.pack.sid);
      ack_idreq=1;  // set ACK for IDREQ frame
      }
    }
  if(packetSize==64)
    { 
    int i=0;
    while (LoRa.available()) { rdf.frame[i] = LoRa.read();i++;}
    if(rdf.pack.con[0]==0x45 && rdf.pack.did==(uint32_t)gwID )  // received DTSUB frame 
      {
      Serial.print("Received DTSUB frame from ");Serial.printf("%08X\n",(uint32_t)rdf.pack.sid);
      xQueueReset(mqttqueue);  // to protect the from overloading
      xQueueSend(mqttqueue, &rdf.pack, portMAX_DELAY);
      ack_dtsub=1; 
 // set ACK for DTPUB frame
      }  
    }
}
    
void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
