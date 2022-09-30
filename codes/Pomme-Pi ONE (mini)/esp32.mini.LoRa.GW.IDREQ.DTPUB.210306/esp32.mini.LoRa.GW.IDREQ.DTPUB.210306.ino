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

RTC_DATA_ATTR int ack_idreq=0, ack_dtpub=0;

void dispTask( void * pvParameters ){
char mess[32]; char dbuff[32];
while(true)
  {
  xQueueReceive(dqueue, mess, portMAX_DELAY);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "MQTT Message");
  display.drawString(0, 16, mess);
  display.display();
  delay(1000);
  }
}


void MQTT_Task( void * pvParameters ){
float stab[8]; char kbuff[32];
MQTTframe_t rdf;
while(true)
  {
  xQueueReceive(mqttqueue, &rdf.pack, portMAX_DELAY);
  LoRa.idle(); 
  Serial.printf("%s\n",rdf.pack.topic);
  Serial.printf("%s\n",rdf.pack.mess);
  if (!client.connected()) { connect(); }
  client.publish("/esp32/my_sensors/", rdf.pack.mess);
  delay(1000);
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
  while (!client.connect("IoT.GW3", "try", "try")) {
    Serial.print(".");
    delay(1000);}
  Serial.printf("IoT.GW3 - connected!\n");

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
  dqueue = xQueueCreate( queueSize, sizeof(float)*8);
  mqttqueue = xQueueCreate( queueSize, sizeof(rdf));
  xTaskCreatePinnedToCore(
                    dispTask,   /* Function to implement the task */
                    "dispTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore);  /* Core where the task should run */
  Serial.println("Task created...");

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
      delay(200);send_ACK(0x32); ack_idreq=0;Serial.println("IDREQ ACK sent");
    }
    
  if (ack_dtpub) 
    { 
      delay(200);send_ACK(0x34); ack_dtpub=0;Serial.println("DTPUB ACK sent");
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
  if(con==0x32) scf.pack.did=(uint32_t)rcf.pack.sid;   // ack for IDREQ frame - mode 3
  if(con==0x34) scf.pack.did=(uint32_t)rdf.pack.sid;   // ack for DTSUB frame 
  scf.pack.sid=(uint32_t)gwID;  // send ACK frame with the received nodeID
  scf.pack.con[0]=con;scf.pack.con[1]=0x00;  // con[0] depends on the type af ACK frame: IDREQ,DTACK,..
  strncpy(scf.pack.pass,"passwordpassword",16);
  scf.pack.tout=20;                     // timeout for the next data frame
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
  Serial.printf("ACK con=%0X,timeout=%d, dest=%08X\n",con,scf.pack.tout,scf.pack.did);
}

void onReceive(int packetSize) 
{
  if(packetSize==32)
    { 
    int i=0;
    while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(rcf.pack.con[0]==0x31 && rcf.pack.did==(uint32_t)0 && !strncmp(rcf.pack.pass,gtpass,16))  // received IDREQ frame for MQTT publish server
      {
      Serial.print("Received IDREQ frame from ");Serial.printf("%08X\n",(uint32_t)rcf.pack.sid);
      ack_idreq=1;  // set ACK for IDREQ frame
      }
    }
  if(packetSize==64)
    { 
    int i=0;
    while (LoRa.available()) { rdf.frame[i] = LoRa.read();i++;}
    if(rdf.pack.con[0]==0x33 && rdf.pack.did==(uint32_t)gwID )  // received DTPUB frame 
      {
      Serial.print("Received DTPUB frame from ");Serial.printf("%08X\n",(uint32_t)rdf.pack.sid);

      xQueueReset(dqueue); xQueueSend(dqueue, rdf.pack.mess, portMAX_DELAY);
      xQueueReset(mqttqueue);  // to protect the from overloading
      xQueueSend(mqttqueue, &rdf.pack, portMAX_DELAY);
      ack_dtpub=1; 
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
