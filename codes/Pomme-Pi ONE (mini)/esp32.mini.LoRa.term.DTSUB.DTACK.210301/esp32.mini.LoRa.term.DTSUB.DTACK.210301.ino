
#include <SPI.h>               
#include <LoRa.h>
#include <Wire.h>  
#include "LoRa_All.h"
             
#include "Adafruit_HTU21DF.h"
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
QueueHandle_t queue;
int queueSize = 32;
static int taskCore = 0;

#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL
uint64_t chipid;  
uint32_t nodeID;  // local ID on 4 bytes
uint16_t timeout=15;
int data_ack=1;
RTC_DATA_ATTR int idreq_ack=0;
RTC_DATA_ATTR uint32_t recvID=0;

conframe_t scf,rcf;              // send control frame , receive control frame
MQTTframe_t sdf,rdf; 

void sensorTask( void * pvParameters ){
float stab[8];
while(true)
  {
  stab[0]=htu.readTemperature();
  stab[1]=htu.readHumidity();
  xQueueReset(queue);
  xQueueSend(queue, stab, portMAX_DELAY);
  delay(2000); 
  Serial.printf("T:%2.2F, H:%2.2f\n",stab[0],stab[1]);
  }
}


void setup() {
Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  nodeID=(uint32_t)chipid; 
  Serial.printf("\nChipID:%08X\n",(uint32_t)nodeID);
  Wire.begin(12,14);
  delay(1000);

  Serial.println("HTU21D-F test");
  if (!htu.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
    }
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
  queue = xQueueCreate(queueSize, sizeof(float)*8);
  xTaskCreatePinnedToCore(
                    sensorTask,   /* Function to implement the task */
                    "sensorTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore);  /* Core where the task should run */
  Serial.println("Task created...");
}

void publish(char *topic, char *mess)
{
  Serial.println("Sending Publish packet: ");
  LoRa_txMode();
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)recvID;    // receiver (GW) ID
  sdf.pack.sid=(uint32_t)nodeID;
  sdf.pack.con[0]=0x07; sdf.pack.con[1]=0x00;       // publish data: 0x07
  strcpy(sdf.pack.topic,topic);   // topic name
  strcpy(sdf.pack.mess,mess);   // topic name
  LoRa.write(sdf.frame,64);
  LoRa.endPacket();
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
    if(data_ack==1)
      {  
      char topic[24],message[24];float stab[8];
      Serial.println("waiting for sensoe task");
      xQueueReceive(queue, stab, portMAX_DELAY);
      strcpy(topic,"/esp32/Term1/Sens1");
      sprintf(message,"T:%2.2f, H:%2.2f",stab[0],stab[1]); 
      publish(topic,message);  Serial.printf("Message published:%s\n",message); 
      Serial.printf("DATA sent to Gateway: %08X\n",recvID);
      data_ack=0;
      }
   }
  }
}

void send_IDREQ()
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  scf.pack.did=(uint32_t)0; scf.pack.sid=(uint32_t)nodeID;
  scf.pack.con[0]=0x01;scf.pack.con[1]=0x00;
  strncpy(scf.pack.pass,"passwordpassword",16);
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send20 it
}



void onReceive(int packetSize) 
{
  if(packetSize==32)
    { 
    int i=0;
    while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(rcf.pack.con[0]==0x02 && rcf.pack.did==nodeID)  // received IDACK frame for this node  
      {
      idreq_ack=1; recvID=rcf.pack.sid; 
      }
           
    if(rcf.pack.con[0]==0x04 && rcf.pack.did==nodeID)  // received DTACK for DTSUB    
      {
      data_ack=1; recvID=rcf.pack.sid; timeout=rcf.pack.tout;
      Serial.printf("Received DTACK\n");
      }
    Serial.printf("con=%0X,did=%08X\n", rcf.pack.con[0], rcf.pack.did);  
    Serial.printf("Terminal:%08X, ACK from Gateway:%08X\n",(uint32_t)nodeID,(uint32_t)recvID);
    Serial.printf("RSSI: %d\n",LoRa.packetRssi());
    }
}
