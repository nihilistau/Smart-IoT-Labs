#define MASTER           // set to chose the IQ mode for MASTER node
//#include <SPI.h>               
#include <LoRa.h>
#include <Wire.h> 
#include "LoRa_Para.h"   // includes the default Lora SPI config and IQ functions for TERMINAL and MASTER
#include "LoRa_Packets.h" // includes the packet formats and AEC functions              
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL

uint64_t chipid;  
uint32_t gwID;
uint32_t termID;

uint16_t timeout;
char *gtpass="passwordpassword";

int ack_idreq=0, ack_dtsnd=0;
float stab[8];
uint8_t mask; int chan; char wkey[16];

QueueHandle_t queue;
int queueSize = 32;
static int taskCore = 0;

void disp_Task( void * pvParameters)
{
 char dbuff[32];float stab[8];
 while(true)
    {
    xQueueReceive(queue, stab, portMAX_DELAY);
     sprintf(dbuff,"%T:%2.2f  H:%2.2f",stab[0],stab[1]);
     display.init();
     display.flipScreenVertically();
     display.setFont(ArialMT_Plain_10);
     display.drawString(0, 0, "Master (1) - DTSND");
     display.drawString(0, 16, dbuff);
     display.display();
    }
}

void setup() {
Serial.begin(9600);
  gwID=(uint32_t)ESP.getEfuseMac();
  //gwID=(uint32_t)chipid;
  //set_LoRa();  // sets the pin config default radio link parameters
  set_LoRa_Para(868e6,125e3,7,0xF3);  // the same as default values
 
  Serial.println("LoRa Link - Master  with TS (DTSND) packet");
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
  queue = xQueueCreate(queueSize, sizeof(float)*8);
  xTaskCreatePinnedToCore(
                    disp_Task,   /* Function to implement the task */
                    "disp_Task", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore);  /* Core where the task should run */
  Serial.println("disp_Task created...");
}
  

void loop() {
  if (ack_idreq) 
    { 
      delay(200);send_IDACK(termID,gwID,0x12,20); ack_idreq=0;
    }
  if (ack_dtsnd) 
    { 
      delay(200);send_DTACK(termID,gwID,mask, chan, wkey,20);ack_dtsnd=0;
      
    }
  delay(100);  
}

void onReceive(int packetSize) 
{
  if(packetSize==32)
    { 
    conframe_t rcf;int i=0;
    while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(rcf.pack.con[0]==0x11 && rcf.pack.did==(uint32_t)0 && !strncmp(rcf.pack.pass,gtpass,16))  // received IDREQ packet
      {
      Serial.print("Received IDREQ frame from ");Serial.printf("%08X\n",(uint32_t)rcf.pack.sid);
      termID=rcf.pack.sid;
      ack_idreq=1;  // set ACK for IDREQ frame
      }
    }
  if(packetSize==64)
    { 
    TSframe_t rdf;int i=0;
    while (LoRa.available()) { rdf.frame[i] = LoRa.read();i++;}
    if(rdf.pack.con[0]==0x13 && rdf.pack.did==(uint32_t)gwID )  // received DTSND packet 
      {
      Serial.print("Received DATA frame from ");Serial.printf("%08X\n",(uint32_t)rdf.pack.sid);
      Serial.printf("Temp:%2.2f, Humi:%2.2f\n",rdf.pack.sens[0],rdf.pack.sens[1]);
      termID=rdf.pack.sid; mask=rdf.pack.con[1];
      xQueueReset(queue);  // reset queue to stor only the last values
      xQueueSend(queue, rdf.pack.sens, portMAX_DELAY);
      ack_dtsnd=1;  // set ACK for DTSND frame
      }  
    }
}
    
