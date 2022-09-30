
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

uint16_t timeout=20, rectimeout=0;
int dtreq_ack=1;
RTC_DATA_ATTR int idreq_ack=0;
RTC_DATA_ATTR uint32_t gwID=0;  // gateway identifier - to be stored in RTC memory
RTC_DATA_ATTR uint8_t count_ack=0;

conframe_t scf,rcf;              // send control frame , receive control frame
TSframe_t sdf,rdf; 


void dispTask( void * pvParameters ){
float stab[8]; char dbuff[32];
while(true)
  {
  xQueueReceive(queue, stab, portMAX_DELAY);
  Serial.printf("S1:%2.2f, S2:%2.2f, S3:%2.2f\n",stab[0],stab[1],stab[2]);
  sprintf(dbuff,"T:%2.2f  H:%2.2f",stab[0],stab[1]);
  display.init();
  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Term Lora - GW 2");
  display.drawString(0, 16, dbuff);
  sprintf(dbuff,"L:%2.2f  CO2:%2.2f",stab[2],stab[3]);
  display.drawString(0, 32, dbuff);
  display.drawString(8, 48, "Smart Computer Lab");
  display.display();
  delay(1000);
  }
}


void setup() {
  Serial.begin(9600);
  delay(1000);
  chipid=ESP.getEfuseMac();
  termID=(uint32_t)chipid;
  Serial.println();  
  Serial.printf("TermID:%08X\n",(uint32_t)termID);
  Wire.begin(12,14);
  delay(1000);
    
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);

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
  queue = xQueueCreate( queueSize, sizeof(float)*8);
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
  if (runEvery(15*1000)) 
    { // repeat every 1000 millis
    if(idreq_ack==0) 
      {
      send_IDREQ(); // send IDREQ control frame
      Serial.println("IDREQ sent");
      }
    else
      {
      if(dtreq_ack==0)
        {
        char *rkey="0XYA1MAWXFGVWDX9";
        int chan=1243348;  // my_channel
        float stab[8];
        uint8_t mask=0xE0;  // field1, field2, field3
        send_DTREQ(mask,chan,rkey); // send DTSND frame
        Serial.printf("DTREQ sent: %08X\n",gwID);
        }
      if(dtreq_ack==1) 
        {
        Serial.println("Received DTREQ ACK"); dtreq_ack=0;
        xQueueSend(queue,rdf.pack.sens, portMAX_DELAY);
        }
      count_ack++;if(count_ack>10)  {idreq_ack =0; count_ack=0; } 
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
  scf.pack.did=(uint32_t)0; scf.pack.sid=(uint32_t)termID;
  scf.pack.con[0]=0x21;scf.pack.con[1]=0x00;   // service 2 - IDREQ frame
  strncpy(scf.pack.pass,"passwordpassword",16);
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}


void send_DTREQ(uint8_t mask, int chan, char *rkey)  // send DTREQ frame
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)gwID;    // receiver (GW) ID
  sdf.pack.sid=(uint32_t)termID;
  sdf.pack.con[0]=0x25; sdf.pack.con[1]=mask;       //service - 2 DTREQ mask
  sdf.pack.channel= chan;
  strncpy(sdf.pack.keyword,rkey,16);
  for(int i=0;i<8;i++) sdf.pack.sens[i]=0.0;    // empty fields
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
    if(rcf.pack.con[0]==0x22 && rcf.pack.did==termID)  // received IDACK service 2  
      {
      idreq_ack=1; gwID=rcf.pack.sid; 
      }
    Serial.printf("Term:%08X, IDACK from Gateway:%08X\n",(uint32_t)rcf.pack.did,(uint32_t)gwID);
    Serial.printf("RSSI: %d\n",LoRa.packetRssi());
    }  
  if(packetSize==64)
    { 
    int i=0;
    while (LoRa.available()) { rdf.frame[i] = LoRa.read();i++;}   
    if(rdf.pack.con[0]==0x26 && rdf.pack.did==termID)  // received DATACK frame for this node  
      {
      dtreq_ack=1; gwID=rdf.pack.sid; rectimeout=rdf.pack.tout; 
      } 
    Serial.printf("Term:%08X, DTRCV ack from Gateway:%08X\n",(uint32_t)rdf.pack.did,(uint32_t)gwID);
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
