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
SemaphoreHandle_t xMutex;
QueueHandle_t dqueue, tsqueue;
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
#define BAND    434E6

int sf=7;
long sbw=125E3;

uint16_t timeout;
char *gtpass="passwordpassword";

conframe_t scf,rcf;  // defined in LoRa_Frames.h
TSframe_t sdf,rdf;

int ack_idreq=0, ack_dtsnd=0;

void dispTask( void * pvParameters ){
float stab[8]; char dbuff[32];
while(true)
  {
  xQueueReceive(dqueue, stab, portMAX_DELAY);
  Serial.printf("Sensor1:%2.2f, Sensor2:%2.2f\n",stab[0],stab[1]);
  sprintf(dbuff,"%T:%2.2f  H:%2.2f",stab[0],stab[1]);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Hello world");
  display.drawString(0, 16, dbuff);
  display.display();
  delay(1000);
  }
}


void TS_Task( void * pvParameters ){
float stab[8]; char kbuff[32];
TSframe_t rdf;
while(true)
  {
  xQueueReceive(tsqueue, &rdf, portMAX_DELAY);
  xSemaphoreTake( xMutex, portMAX_DELAY );
  if(rdf.pack.con[1] & 0x80) ThingSpeak.setField(1, rdf.pack.sens[0]);
  if(rdf.pack.con[1] & 0x40) ThingSpeak.setField(2, rdf.pack.sens[1]);
  if(rdf.pack.con[1] & 0x20) ThingSpeak.setField(3, rdf.pack.sens[2]);
  if(rdf.pack.con[1] & 0x10) ThingSpeak.setField(4, rdf.pack.sens[3]);
  if(rdf.pack.con[1] & 0x08) ThingSpeak.setField(5, rdf.pack.sens[4]);
  if(rdf.pack.con[1] & 0x04) ThingSpeak.setField(6, rdf.pack.sens[5]);
  if(rdf.pack.con[1] & 0x02) ThingSpeak.setField(7, rdf.pack.sens[6]);
  if(rdf.pack.con[1] & 0x01) ThingSpeak.setField(8, rdf.pack.sens[7]);

  Serial.println(rdf.pack.con[1]);
  Serial.println(rdf.pack.keyword);
  Serial.println(rdf.pack.sens[0]);
  Serial.println(rdf.pack.sens[1]);

  memset(kbuff,0x00,32); strncpy(kbuff,rdf.pack.keyword,16);
  
  // write to the ThingSpeak channel
  int x = ThingSpeak.writeFields((uint32_t)rdf.pack.channel,kbuff); // "J4K8ZIWAWE8JBIX7"
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  xSemaphoreGive( xMutex );
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
  gwID=(uint32_t)chipid;

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
  Serial.println("Starting Gateway - mode 1");
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
  tsqueue = xQueueCreate( queueSize, sizeof(rdf));
  xMutex = xSemaphoreCreateMutex();
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
  if (ack_idreq) 
    { 
      send_ACK(0x12); ack_idreq=0;
    }
  if (ack_dtsnd) 
    { 
      send_ACK(0x14); ack_dtsnd=0;
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
  if(con==0x12) scf.pack.did=(uint32_t)rcf.pack.sid;  // ack for IDREQ frame
  if(con==0x14) scf.pack.did=(uint32_t)rdf.pack.sid;  // ack for DTSND frame 
  scf.pack.sid=(uint32_t)gwID;  // send ACK frame with the received nodeID
  scf.pack.con[0]=con;scf.pack.con[1]=0x00;  // con[0] depends on the type af ACK frame: IDREQ,DTACK,..
  strncpy(scf.pack.pass,"passwordpassword",16);
  scf.pack.tout=20;                     // timeout for the next data frame
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
  Serial.printf("ACK sent with con=%0X and timeout=%d\n",con,scf.pack.tout);
}

void onReceive(int packetSize) 
{
xSemaphoreTake( xMutex, portMAX_DELAY );
  if(packetSize==32)
    { 
    int i=0;
    while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(rcf.pack.con[0]==0x11 && rcf.pack.did==(uint32_t)0 && !strncmp(rcf.pack.pass,gtpass,16))  // received IDREQ frame for TS send server
      {
      Serial.print("Received IDREQ frame from ");Serial.printf("%08X\n",(uint32_t)rcf.pack.sid);
      ack_idreq=1;  // set ACK for IDREQ frame
      }
    }
  if(packetSize==64)
    { 
    int i=0;
    while (LoRa.available()) { rdf.frame[i] = LoRa.read();i++;}
    if(rdf.pack.con[0]==0x13 && rdf.pack.did==(uint32_t)gwID )  // received DATA frame 
      {
      Serial.print("Received DATA frame from ");Serial.printf("%08X\n",(uint32_t)rdf.pack.sid);
      xQueueReset(dqueue); xQueueSend(dqueue, rdf.pack.sens, portMAX_DELAY);
      xQueueReset(tsqueue);  // to protect the from overloading
      xQueueSend(tsqueue, &rdf.pack, portMAX_DELAY);
      ack_dtsnd=1;  // set ACK for DTSND frame
      }  
    }
  xSemaphoreGive( xMutex );
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
