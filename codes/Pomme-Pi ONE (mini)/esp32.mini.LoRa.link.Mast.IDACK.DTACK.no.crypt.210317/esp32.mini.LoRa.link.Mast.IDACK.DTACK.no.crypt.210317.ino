
#include <SPI.h>               
#include <LoRa.h>
#include <Wire.h> 
#include "LoRa_Para.h"   // includes the default Lora config and IQ functions 
#include "LoRa_Packets.h" // includes the packet formats and AEC functions              
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL


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

conframe_t scf,rcf,rccf;  // control sens and receive, receive crypted packets
TSframe_t sdf,rdf,rdcf;   // data sens and receive, receive data crypted packets

int ack_idreq=0, ack_dtsnd=0;
float stab[8];

void disp_Fun()
{
 char dbuff[32];

  Serial.printf("Sensor1:%2.2f, Sensor2:%2.2f\n",rdf.pack.sens[0],rdf.pack.sens[1]);
  sprintf(dbuff,"%T:%2.2f  H:%2.2f",rdf.pack.sens[0],rdf.pack.sens[1]);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Master (1) - DTSND");
  display.drawString(0, 16, dbuff);
  display.display();
  delay(1000);
}

void setup() {
Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  gwID=(uint32_t)chipid;

  
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);

  Serial.begin(9600);
  delay(1000);
  Serial.println();Serial.println();
  Serial.println("Starting Master - mode 1");
  Serial.printf("%08X\n",(uint32_t)chipid);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Starting LoRa physical parameters");
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sbw);
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Link - Master  with TS (DTSND) packet");
  Serial.println("Only receive messages from nodes");
  Serial.println("Tx: invertIQ enable");
  Serial.println("Rx: invertIQ disable");
  Serial.println();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}
  

void loop() {
  if (ack_idreq) 
    { 
      delay(200);send_ACK(0x12); ack_idreq=0;
    }
  if (ack_dtsnd) 
    { 
      delay(200);send_ACK(0x14); disp_Fun();ack_dtsnd=0;
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

uint16_t calc_Timeout(uint32_t tid)  // terminal identifier
{
 return (uint16_t) random(10,30); 
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
  scf.pack.tout=calc_Timeout(rcf.pack.sid);               // calculate timeout for the next data frame
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
  Serial.printf("ACK con=%0X,timeout=%d, dest=%08X\n",con,scf.pack.tout,scf.pack.did);
}

void onReceive(int packetSize) 
{
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
      Serial.printf("Temp:%2.2f, Humi:%2.2f\n",rdf.pack.sens[0],rdf.pack.sens[1]);
      ack_dtsnd=1;  // set ACK for DTSND frame
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
