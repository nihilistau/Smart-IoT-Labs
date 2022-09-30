#define MASTER  // to choose TERMINAL or MASTER (GATEWAY) node
#define TS      // to choose TS (ThingSpeak) or MQTT
#define MODE 1  // to choose sender (1), receiver (2) , publisher (3), subscriber (4) mode
// 
#include <SPI.h>               
#include <LoRa.h> 
#include "LoRa_Para.h"
#include "LoRa_Packets.h"
#include "LoRa_onReceive.h"  // to capture the packets with MODE and server/broker

//RTC_DATA_ATTR  uint32_t termID,gwID;
//int stage1_flag=0;  // reception flag for control packets: GW-IDREQ, TERM-IDACK
//int stage2_flag=0;  // reception flag for data packets: TS-GW-DTSND, TS-TERM-DTACK; TS-GW-DTREQ,TS-TERM-DTRCV 
// reception flag for data packets: MQTT-GW-DTPUB, MQTT-TERM-DTSUBACK; MQTT-GW-DTPUB, MQTT-TERM-DTSUBRCV

void setup() {
  // here you choose the initialization functions:
  // void set_LoRa() -  all default settings
  // void set_LoRa_Pins_Para(unsigned long freq,unsigned sbw, int sf, uint8_t sw) - radio user settings
  // void set_LoRa_Para(int sck,int miso,int mosi,int ss,int rst, int dio0,unsigned long freq,unsigned sbw,int sf,uint8_t sw) 
  // example settings:
  Serial.begin(9600); 
  gwID=(uint32_t)ESP.getEfuseMac();                  
  set_LoRa();
  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}

void loop() {
  // here you use the stage1 and stage2 flags to detect the arrival control and data packets
  // then you use the predefined send functions to send the packets: 
  // void send_IDREQ(uint32_t termID,uint8_t serv, char *pass, char *crypt)

  // void send_DTSND(uint32_t gwID,uint32_t termID,uint8_t mask,int chan,char *wkey,float *stab, char *crypt)
  // void send_DTREQ(uint32_t gwID,uint32_t termID,uint8_t mask,int chan,char *rkey, char *crypt)
  // void send_DTPUB(uint32_t gwID,uint32_t termID,char *topic, char *mess, char *crypt)
  // void send_DTPUBACK(uint32_t gwID,uint32_t termID,char *topic,char *mess, char *crypt)
  // void send_DTSUB(uint32_t gwID,uint32_t termID,char *topic, char *crypt)
  // void send_IDACK(uint32_t termID,uint32_t gwID,uint8_t serv,uint16_t tout, char *crypt)
  // void send_DTACK(uint32_t termID,uint32_t gwID,uint8_t mask,int chan,char *wkey,uint16_t tout, char *crypt)
  // void send_DTRCV(uint32_t termID,uint32_t gwID,uint8_t mask,int chan,char *rkey,float stab[],uint16_t tout, char *crypt) 
  // void send_DTSUBRCV(uint32_t gwID,uint32_t termID,char *topic,char *mess, char *crypt) 

  if(stage1_flag==1) 
  {
  // MODE 1: IDACK - 0x12 : mode*16+2
  send_IDACK(termID,gwID,MODE,20,NULL); // send IDACK for service 1, NULL - no AES
  Serial.printf("IDACK service=%x sent\n",MODE);stage1_flag=0;
  }
  

}
