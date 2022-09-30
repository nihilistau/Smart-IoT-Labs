

#include <SPI.h>               
#include <LoRa.h>
#include <Wire.h> 
#include "LoRa_Frames.h"               

QueueHandle_t queue;
int queueSize = 32;
static int taskCore = 0;

 
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL

uint64_t chipid;  
uint32_t termID;  // local ID on 4 bytes

#define SS       5 // 26     // D0 - to NSS
#define RST     15  //16     // D4  - RST
#define DI0     26      // D8 - INTR
#define SCK     18      // D5 - CLK
#define MISO    19      // D6 - MISO
#define MOSI    23      // D7 - MOSI
#define BAND    868E6

int sf=7;
long sbw=125E3;

uint16_t timeout=10;
RTC_DATA_ATTR int idreq_ack=0,dtrcv_ack=0;
RTC_DATA_ATTR uint32_t gwID=0;  // gateway identifier - to be stored in RTC memory
RTC_DATA_ATTR uint8_t count_ack=0,count_sub=0;

conframe_t scf,rcf;              // send control frame , receive control frame
MQTTframe_t sdf,rdf; 


void dispTask( void * pvParameters ){
char mess[32]; char dbuff[32]; MQTTframe_t buff;
while(true)
  {
  xQueueReceive(queue, &buff, portMAX_DELAY);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "MQTT Topic/Message");
  display.drawString(0, 16, buff.pack.topic);
  display.drawString(0, 32, buff.pack.mess);
  display.display();
  delay(1000);
  }
}


void setup() {
Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  termID=(uint32_t)chipid; 
  Serial.printf("\nChipID:%08X\n",(uint32_t)termID);
  Wire.begin(12,14);
    
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
  queue = xQueueCreate(queueSize,64);  // max size of message
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
  
  if (runEvery(10*1000)) 
    { // repeat every 1000 millis
    if(idreq_ack==0) 
      {
      send_IDREQ(); // send IDREQ control frame
      Serial.println("IDREQ sent");
      }
    else
      {
      if(dtrcv_ack==0)
        {
        char *topic="/esp32/my_sensors/"; 
        send_DTSUB(topic); // send DTSND frame
        Serial.printf("DTSUB to: %08X with topic:%s\n",gwID,topic);
        dtrcv_ack=0;
        }
      if(dtrcv_ack==1) 
        {
          Serial.println("Received DTSUB ACK"); dtrcv_ack=0;  
        }
       count_ack++;if(count_ack>10)  {idreq_ack =0; count_ack=0; }   // restart IDREQ
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

void send_IDREQ()  // mode MQTT - 3
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  scf.pack.did=(uint32_t)0; scf.pack.sid=(uint32_t)termID;
  scf.pack.con[0]=0x41;scf.pack.con[1]=0x00;   // service 4 - IDREQ frame
  strncpy(scf.pack.pass,"passwordpassword",16);
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}


void send_DTSUB(char *topic)  // send DTSUB frame - subscribe topic
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)gwID;    // receiver (GW) ID
  sdf.pack.sid=(uint32_t)termID;
  sdf.pack.con[0]=0x43; sdf.pack.con[1]=0x00;        
  strncpy(sdf.pack.topic,topic,24);
  memset(sdf.pack.mess,0x00,24);
  sdf.pack.tout=0;
  LoRa.write(sdf.frame,64);
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) 
{
  if(packetSize==32)
    { 
    int i=0;
    while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(rcf.pack.con[0]==0x42 && rcf.pack.did==termID)  // received IDACK - mode 4 - MQTT frame for this node  
      {
      idreq_ack=1; gwID=rcf.pack.sid; 
      }
//    if(rcf.pack.con[0]==0x46 && rcf.pack.did==termID)  // received DTSUB ACK - mode 4 - MQTT frame for this node  
//      {
//      dtsub_ack=1; gwID=rcf.pack.sid; 
//      }
    Serial.printf("Received - Term:%08X, con=%02X, ACK from:%08X\n",(uint32_t)rcf.pack.did,rcf.pack.con[0],(uint32_t)gwID);
    }
  if(packetSize==64)
    { 
    int i=0;
    while (LoRa.available()) { rdf.frame[i] = LoRa.read();i++;} 
    if(rdf.pack.con[0]==0x44 && rdf.pack.did==termID && !strncmp(rdf.pack.topic,sdf.pack.topic,24))   // received DTPUB ACK frame for this node  
      {
      Serial.printf("Received - Term:%08X, con=%02X, ACK from:%08X\n",(uint32_t)rdf.pack.did,rdf.pack.con[0],(uint32_t)gwID);
      dtrcv_ack=1; gwID=rdf.pack.sid; timeout=rdf.pack.tout;
      xQueueReset(queue);  // to protect the from overloading
      xQueueSend(queue, &rdf, portMAX_DELAY);
      }
    }
  Serial.printf("RSSI: %d\n",LoRa.packetRssi());
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
