#include <WiFi.h>
#include "ThingSpeak.h" 
#include "LoRa_Frames.h"
#include <SPI.h>               
#include <LoRa.h>
#include <Wire.h>                
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL

const char* ssid = "Livebox-08B0";
const char* pass = "G79ji6dtEptVTPWmZP";

WiFiClient client;

QueueHandle_t tsqueue;
int queueSize = 32;
static int taskCore = 0;

uint64_t chipid;  
uint32_t nodeID;
uint32_t recvID;

#define SS       5 // 26     // D0 - to NSS
#define RST     15  //16     // D4  - RST
#define DI0     26      // D8 - INTR

#define SCK     18      // D5 - CLK
#define MISO    19      // D6 - MISO
#define MOSI    23      // D7 - MOSI
#define BAND    434E6

int sf=7;
long sbw=125E3;

uint16_t timeout;
char *gtpass="passwordpassword";

conframe_t scf,rcf;  // defined in LoRa_Frames.h
TSframe_t sdf,rdf;

void TS_Task( void * pvParameters ){
float stab[8]; char kbuff[32];
TSframe_t r;  float tsens[8];
while(true)
  {
  xQueueReceive(tsqueue, &r, portMAX_DELAY);
  memset(kbuff,0x00,32); strncpy(kbuff,r.pack.keyword,16);
  if(r.pack.con[1] & 0x80) { tsens[0]=ThingSpeak.readFloatField(r.pack.channel,1,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x40) { tsens[1]=ThingSpeak.readFloatField(r.pack.channel,2,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x20) { tsens[2]=ThingSpeak.readFloatField(r.pack.channel,3,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x10) { tsens[3]=ThingSpeak.readFloatField(r.pack.channel,4,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x08) { tsens[4]=ThingSpeak.readFloatField(r.pack.channel,5,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x04) { tsens[5]=ThingSpeak.readFloatField(r.pack.channel,6,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x02) { tsens[6]=ThingSpeak.readFloatField(r.pack.channel,7,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x01) { tsens[7]=ThingSpeak.readFloatField(r.pack.channel,8,kbuff);delay(2000); }
  send_DATA(0x06,tsens);Serial.printf("DTSND:%2.2f,%2.2f\n",tsens[0],tsens[1]);
  }
}

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nIoT.GW1 - connected!");
}


void setup() {
Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  nodeID=(uint32_t)chipid;

  WiFi.begin(ssid, pass);
  if (!client.connected()) { connect(); }
  Serial.println();Serial.println();
  Serial.println("WiFi connected");
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);

  Serial.begin(9600);
  delay(1000);
  Serial.println();Serial.println();
  Serial.println("Starting LoRa Sender");
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
  tsqueue = xQueueCreate( queueSize, sizeof(rdf));      // received frame - DTREQ to TS
  xTaskCreatePinnedToCore(
                    TS_Task,   /* Function to implement the task */
                    "TS_Task", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore);  /* Core where the task should run */
  Serial.println("Task created...");
}

void loop() {
  if (runEvery(5000)) { // repeat every 5000 millis
  Serial.println("In the gateway");
  }
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
  if(con==0x02) scf.pack.did=(uint32_t)rcf.pack.sid;  // ack for control frame
  if(con==0x04) scf.pack.did=(uint32_t)rdf.pack.sid;  // ack for data frame
  scf.pack.sid=(uint32_t)nodeID;  // send ACK frame with the received nodeID
  scf.pack.con[0]=con;scf.pack.con[1]=0x00;  // con[0] depends on the type af ACK frame: IDREQ,DTACK,..
  strncpy(scf.pack.pass,"passwordpassword",16);
  scf.pack.tout=20;                     // timeout for the next data frame
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
  Serial.printf("ACK sent with con=%0X and timeout=%d\n",con,scf.pack.tout);
}

void send_DATA(uint8_t con, float *tsens)             // ACK frame with specific control field
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  if(con==0x06) sdf.pack.did=(uint32_t)rdf.pack.sid;  // ack for data frame
  sdf.pack.sid=(uint32_t)nodeID;  // send ACK frame with the received nodeID
  sdf.pack.con[0]=con;sdf.pack.con[1]=rdf.pack.con[1];  // con[0] depends on the type af ACK frame: IDREQ,DTACK,..
  memcpy(sdf.pack.sens,tsens,32);
  sdf.pack.tout=20;                     // timeout for the next data frame
  LoRa.write(sdf.frame,64);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
  Serial.printf("ACK sent with con=%0X and timeout=%d\n",con,scf.pack.tout);
}


void onReceive(int packetSize) 
{
  if(packetSize==32)
    { 
    int i=0;
    while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(rcf.pack.con[0]==0x01 && rcf.pack.did==(uint32_t)0 && !strncmp(rcf.pack.pass,gtpass,16))  // received IDREQ frame
  //  if(rcf.pack.con[0]==0x11 && rcf.pack.did==(uint32_t)0 && !strncmp(rcf.pack.pass,gtpass,16))  // received IDREQ frame for TS send server
      {
      Serial.print("Received IDREQ frame from ");Serial.printf("%08X\n",(uint32_t)rcf.pack.sid);
      Serial.print("Gateway ID ");Serial.printf("%08X\n",(uint32_t)nodeID);
      send_ACK(0x02);
      }
    }
  if(packetSize==64)
    { 
    int i=0;
    while (LoRa.available()) { rdf.frame[i] = LoRa.read();i++;}
    if(rdf.pack.con[0]==0x05 && rdf.pack.did==(uint32_t)nodeID )  // received DATA frame 
      {
      Serial.print("Received DTREQ frame from ");Serial.printf("%08X\n",(uint32_t)rdf.pack.sid);
      xQueueReset(tsqueue);
      xQueueSend(tsqueue, &rdf, portMAX_DELAY);
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
