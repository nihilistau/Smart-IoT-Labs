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
QueueHandle_t queue;
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
TSframe_t sdf,rdf;

int ack_idreq=0, ack_dtreq=0;


void TS_Task( void * pvParameters ){
float stab[8]; char kbuff[32];
TSframe_t r;  // received data req frame
while(true)
  {
  xQueueReceive(queue, &r, portMAX_DELAY);
  LoRa.idle(); Serial.println("In TS task");
  memset(kbuff,0x00,32); strncpy(kbuff,r.pack.keyword,16);
  if(r.pack.con[1] & 0x80) { sdf.pack.sens[0]=ThingSpeak.readFloatField(r.pack.channel,1,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x40) { sdf.pack.sens[1]=ThingSpeak.readFloatField(r.pack.channel,2,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x20) { sdf.pack.sens[2]=ThingSpeak.readFloatField(r.pack.channel,3,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x10) { sdf.pack.sens[3]=ThingSpeak.readFloatField(r.pack.channel,4,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x08) { sdf.pack.sens[4]=ThingSpeak.readFloatField(r.pack.channel,5,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x04) { sdf.pack.sens[5]=ThingSpeak.readFloatField(r.pack.channel,6,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x02) { sdf.pack.sens[6]=ThingSpeak.readFloatField(r.pack.channel,7,kbuff);delay(2000); }
  if(r.pack.con[1] & 0x01) { sdf.pack.sens[7]=ThingSpeak.readFloatField(r.pack.channel,8,kbuff);delay(2000); }
  Serial.printf("%2.2f,%2.2f,%2.2f\n",sdf.pack.sens[0],sdf.pack.sens[1],sdf.pack.sens[2]);
  delay(1000);
  LoRa_rxMode();ack_dtreq=1;
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

  queue = xQueueCreate(queueSize, sizeof(rdf));


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
      delay(200);send_ACK(0x22); ack_idreq=0;
    }
  if (ack_dtreq) 
    { 
      delay(200);send_DTRCV(0x26); ack_dtreq=0;
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
  if(con==0x22) scf.pack.did=(uint32_t)rcf.pack.sid;  // ack for IDREQ frame
  scf.pack.sid=(uint32_t)gwID;  // send ACK frame with the received nodeID
  scf.pack.con[0]=0x22;scf.pack.con[1]=0x00;  // con[0] : IDREQ - mode 2,
  strncpy(scf.pack.pass,"passwordpassword",16);
  scf.pack.tout=20;                     // timeout for the next data frame
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
  Serial.printf("ACK con=%0X,timeout=%d, dest=%08X\n",con,scf.pack.tout,scf.pack.did);
}

void send_DTRCV(uint8_t con)             // DTRCV frame - mode 2
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  sdf.pack.did=(uint32_t)rdf.pack.sid;  // ack for DTSND frame 
  sdf.pack.sid=(uint32_t)gwID;          // gateway ID
  sdf.pack.con[0]=con; sdf.pack.con[1]=rdf.pack.con[1]; // put the mask field
  sdf.pack.tout=20;                     // timeout for the next data frame
  LoRa.write(sdf.frame,64);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
  Serial.printf("DTRCV con=%0X,timeout=%d, dest=%08X\n",con,sdf.pack.tout,sdf.pack.did);
}


void onReceive(int packetSize) 
{
  if(packetSize==32)
    { 
    int i=0;
    while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(rcf.pack.con[0]==0x21 && rcf.pack.did==(uint32_t)0 && !strncmp(rcf.pack.pass,gtpass,16))  // received IDREQ mode 2
      {
      Serial.print("Received IDREQ frame from ");Serial.printf("%08X\n",(uint32_t)rcf.pack.sid);
      ack_idreq=1;  // set ACK for IDREQ frame
      }
    }
  if(packetSize==64)
    { 
    int i=0;
    while (LoRa.available()) { rdf.frame[i] = LoRa.read();i++;}
    if(rdf.pack.con[0]==0x25 && rdf.pack.did==(uint32_t)gwID )  // received DTREQ - mode 2
      {
      Serial.print("Received DTREQ frame from ");Serial.printf("%08X\n",(uint32_t)rdf.pack.sid);
      xQueueReset(queue);  // to protect the from overloading
      xQueueSend(queue,&rdf,portMAX_DELAY);
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
