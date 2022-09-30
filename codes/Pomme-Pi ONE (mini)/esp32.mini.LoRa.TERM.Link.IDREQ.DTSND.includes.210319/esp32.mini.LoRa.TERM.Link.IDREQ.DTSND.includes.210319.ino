#define TERMINAL
#include <SPI.h>               
#include <LoRa.h>
#include <Wire.h> 
#include "LoRa_Para.h"   // includes the default Lora config and IQ functions 
#include "LoRa_Packets.h" // includes the packet formats and AEC functions           
#include "SHT21.h"

SHT21 SHT21;

uint64_t chipid;  
uint32_t termID;  // local ID on 4 bytes
uint16_t timeout=10, tout;
int dtsnd_ack=0;
RTC_DATA_ATTR int idreq_ack=0;
RTC_DATA_ATTR uint32_t gwID=0;  // gateway identifier - to be stored in RTC memory
RTC_DATA_ATTR uint8_t count_ack=0;

//conframe_t scf,rcf,sccf;              // send - receive control packet
//TSframe_t sdf,rdf,sdcf;               // send - receive data packet

QueueHandle_t queue;
int queueSize = 32;
static int taskCore = 0;

void sensor_Task( void * pvParameters )
{
  float stab[8];
  while(true)
    {
    SHT21.begin(); delay(200);
    stab[0]=(float) SHT21.getTemperature();
    stab[1]=(float) SHT21.getHumidity();
    delay(timeout*1000/2);
    xQueueReset(queue);  // reset queue to stor only the last values
    xQueueSend(queue, stab, portMAX_DELAY);
    }
}


void setup() {
Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  termID=(uint32_t)chipid; 
  Serial.printf("\nChipID:%08X\n",(uint32_t)termID);
  Wire.begin(12,14);
  delay(1000);
  SHT21.begin();
  set_LoRa();  // sets the pin config default radio link parameters
  Serial.println("Tx: invertIQ disable - Rx: invertIQ enable");  
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
  Serial.println(taskCore);
  queue = xQueueCreate( queueSize, sizeof(float)*8);
  xTaskCreatePinnedToCore(
                    sensor_Task,   /* Function to implement the task */
                    "sensor_Task", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore);  /* Core where the task should run */
  Serial.println("Task created...");
}

void loop() 
{
  if (runEvery(timeout*1000)) 
    {  
    if(idreq_ack==0) 
      {
      uint8_t serv=0x11;
      send_IDREQ(termID,serv); // send IDREQ for service 1
      Serial.printf("IDREQ service=%x sent\n",serv);
      }
    else
      {
      if(dtsnd_ack==0)
        {
        float stab[8];
        uint8_t mask=0xC0; int chan=123456; char *wkey="abcdefgijklmnopq";
        xQueueReceive(queue, stab, portMAX_DELAY);
        Serial.print("temp:");Serial.println(stab[0]);
        Serial.print("humi:");Serial.println(stab[1]);
        send_DTSND(gwID,termID,mask,chan,wkey,stab); // send DTSND frame
        Serial.printf("DTSND: %08X\n",gwID);
        }
      if(dtsnd_ack==1) 
        {
         Serial.println("Received DTACK"); dtsnd_ack=0; 

        }
       count_ack++;if(count_ack>10)  {idreq_ack =0; count_ack=0; }   // restart IDREQ
      }
    }
}

void onReceive(int packetSize) // service 1 - TS sender
{
  if(packetSize==32)
    { 
    conframe_t rcf; int i=0;
    while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(rcf.pack.con[0]==0x12 && rcf.pack.did==termID)  // received IDACK packet
      {
      idreq_ack=1; gwID=rcf.pack.sid; Serial.println("Received IDACK");
      }
    }
  if(packetSize==64)
    { 
    TSframe_t rdf; int i=0; 
    while (LoRa.available()) { rdf.frame[i] = LoRa.read();i++;}  
    if(rdf.pack.con[0]==0x14 && rdf.pack.did==termID)  // received DTACK packet
      {
      dtsnd_ack=1; gwID=rdf.pack.sid; tout=rdf.pack.tout;Serial.println("Received DTACK");
      } 
    }  
}
