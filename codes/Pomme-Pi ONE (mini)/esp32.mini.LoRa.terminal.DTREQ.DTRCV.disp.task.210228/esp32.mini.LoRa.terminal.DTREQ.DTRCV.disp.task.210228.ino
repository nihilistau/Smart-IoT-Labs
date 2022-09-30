
#include <SPI.h>               
#include <LoRa.h>
#include <Wire.h> 
#include "LoRa_Frames.h"  
             
QueueHandle_t dqueue;
int queueSize = 32;
static int taskCore = 0;

 
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL


uint64_t chipid;  
uint32_t nodeID;  // local ID on 4 bytes
//uint32_t recvID;  // gateway ID

#define SS       5 // 26     // D0 - to NSS
#define RST     15  //16     // D4  - RST
#define DI0     26      // D8 - INTR

#define SCK     18      // D5 - CLK
#define MISO    19      // D6 - MISO
#define MOSI    23      // D7 - MOSI
#define BAND    434E6

int sf=7;
long sbw=125E3;

uint16_t timeout=15;
int data_ack=0;
RTC_DATA_ATTR int idreq_ack=0;
RTC_DATA_ATTR uint32_t recvID=0;

conframe_t scf,rcf;              // send control frame , receive control frame
TSframe_t sdf,rdf; 

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
  delay(4000);
  }
}

void setup() {
Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  nodeID=(uint32_t)chipid; 
  Serial.printf("\nChipID:%08X\n",(uint32_t)nodeID);
  Wire.begin(12,14);
  delay(1000);
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);
  Serial.begin(9600);
  delay(1000);
  Serial.println();Serial.println();
  Serial.printf("%08X\n",(uint32_t)chipid);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Starting LoRa ok!");
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sbw); 
  LoRa.setSyncWord(0xF3);           // ranges from 0-0xFF, default 0x34, see API docs
  Serial.println("Tx: invertIQ disable - Rx: invertIQ enable");  
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
  Serial.println(taskCore);
  dqueue = xQueueCreate(queueSize, sizeof(float)*8);
  xTaskCreatePinnedToCore(
                    dispTask,   /* Function to implement the task */
                    "dispTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore);  /* Core where the task should run */
  Serial.println("Task created...");
}

void loop() 
{
  if (runEvery(10000)) 
    {  
    if(idreq_ack==0) 
      {
      send_IDREQ(); // send IDREQ control frame
      Serial.println("IDREQ sent");
      }
    else
    {
    if(data_ack==0)
      {  
      char *rkey="0XYA1MAWXFGVWDX9";   // read key in ThingSpeak channel
      int chan=1243348;  // my_channel
      float stab[8];
      uint8_t mask=0xC0;  
      req_DATA(mask,chan,rkey); // request DATA frame: DTREQ
      Serial.printf("DATA sent to Gateway: %08X\n",recvID);
      }
    if(data_ack==1) 
    {
    xQueueReset(dqueue);
    xQueueSend(dqueue, rdf.pack.sens, portMAX_DELAY);  // sensor values from queue stored in stab
    Serial.printf("DATA ACK received\n");data_ack=0;
    }
   }
  }
}

void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

void send_IDREQ()
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  scf.pack.did=(uint32_t)0; scf.pack.sid=(uint32_t)nodeID;
  scf.pack.con[0]=0x01;scf.pack.con[1]=0x00;
  strncpy(scf.pack.pass,"passwordpassword",16);
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}


void get_DATA(float *stab)
{
for(int i=0;i<8;i++) sdf.pack.sens[i]=stab[i];
}

void req_DATA(uint8_t mask, int chan, char *rkey)
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)recvID;    // receiver (GW) ID
  sdf.pack.sid=(uint32_t)nodeID;
  sdf.pack.con[0]=0x05; sdf.pack.con[1]=mask;       // DTREQ 0x05 or 0x25 send request for data
  sdf.pack.channel= chan;
  strncpy(sdf.pack.keyword,rkey,16);
  sdf.pack.tout=0;
  LoRa.write(sdf.frame,64);
  LoRa.endPacket(true);                 // finish packet and send it
  Serial.println("DTREQ sent");
}

void onReceive(int packetSize) 
{
  if(packetSize==32)
    { 
    int i=0;
    while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(rcf.pack.con[0]==0x02 && rcf.pack.did==nodeID)  // received ACK_ID frame for this node  
      {
      idreq_ack=1; recvID=rcf.pack.sid; 
      }
    Serial.printf("Terminal:%08X, ACK from Gateway:%08X\n",(uint32_t)nodeID,(uint32_t)recvID);
    Serial.printf("RSSI: %d\n",LoRa.packetRssi());  
    }
  if(packetSize==64)
    { 
    int i=0;
    while (LoRa.available()) { rdf.frame[i] = LoRa.read();i++;}       
    if(rdf.pack.con[0]==0x06 && rdf.pack.did==nodeID)  // received DTRCV  frame for this node  
      {
      data_ack=1; recvID=rdf.pack.sid; timeout=rdf.pack.tout;
      xQueueReset(dqueue);
      xQueueSend(dqueue, rdf.pack.sens, portMAX_DELAY);  // sensor values sent to dqueue
      Serial.printf("DRRCV: %2.2f, %2.2f\n",rdf.pack.sens[0],rdf.pack.sens[1]);
      }
    Serial.printf("con=%0X,did=%08X\n", rdf.pack.con[0], rdf.pack.did);  
    Serial.printf("Terminal:%08X, ACK from Gateway:%08X\n",(uint32_t)nodeID,(uint32_t)recvID);
    Serial.printf("RSSI: %d\n",LoRa.packetRssi());
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
