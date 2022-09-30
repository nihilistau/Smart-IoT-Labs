
#include <SPI.h>               
#include <LoRa.h>
#include <Wire.h> 
#include "LoRa_Frames.h"               
#include "Adafruit_HTU21DF.h"
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

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
#define BAND    434E6

int sf=7;
long sbw=125E3;

uint16_t timeout=10;
int dtsnd_ack=1;
RTC_DATA_ATTR int idreq_ack=0;
RTC_DATA_ATTR uint32_t gwID=0;  // gateway identifier - to be stored in RTC memory
RTC_DATA_ATTR uint8_t count_ack=0;

conframe_t scf,rcf;              // send control frame , receive control frame
TSframe_t sdf,rdf; 


void sensorTask( void * pvParameters ){
float stab[8];
while(true)
  {
  htu.begin(); delay(200);
  stab[0]=htu.readTemperature();
  stab[1]=htu.readHumidity();
  delay(timeout*1000/2);
  xQueueReset(queue);
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
  queue = xQueueCreate( queueSize, sizeof(float)*8);
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

long last_ack=0;

void loop() 
{
  if (runEvery(timeout*1000)) 
    { // repeat every 1000 millis
    if(idreq_ack==0) 
      {
      send_IDREQ(); // send IDREQ control frame
      Serial.println("IDREQ sent");
      }
    else
      {
      if(dtsnd_ack==0)
        {
        char *wkey="J4K8ZIWAWE8JBIX7";
        int chan=1243348;  // my_channel
        float stab[8];
        uint8_t mask=0xC0;  
        xQueueReceive(queue, stab, portMAX_DELAY);
        Serial.print("temp:");Serial.println(stab[0]);
        Serial.print("humi:");Serial.println(stab[1]);
        send_DTSND(mask,chan,wkey,stab); // send DTSND frame
        Serial.printf("DTSND: %08X\n",gwID);
        }
      if(dtsnd_ack==1) 
        {
          Serial.println("Received DTACK"); dtsnd_ack=0;  
//        LoRa.end();  // stops LoRa modem and SPI bus connection
//        if(timeout==0) timeout=15;   // if received timeout is 0 set 15 secondes
//        Serial.printf("Enters deep sleep for: %d sec\n",timeout); 
//        esp_sleep_enable_timer_wakeup(1000*1000*timeout);  // in micro-seconds - timeout in seconds
//        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); delay(100);
//        esp_deep_sleep_start(); 
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

void send_IDREQ()
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  scf.pack.did=(uint32_t)0; scf.pack.sid=(uint32_t)termID;
  scf.pack.con[0]=0x11;scf.pack.con[1]=0x00;   // service 1 - IDREQ frame
  strncpy(scf.pack.pass,"passwordpassword",16);
  LoRa.write(scf.frame,32);             // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}


void send_DTSND(uint8_t mask, int chan, char *wkey, float *stab)  // send DTSND frame
{
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)gwID;    // receiver (GW) ID
  sdf.pack.sid=(uint32_t)termID;
  sdf.pack.con[0]=0x13; sdf.pack.con[1]=mask;       // sendr data with mask
  sdf.pack.channel= chan;
  strncpy(sdf.pack.keyword,wkey,16);
  for(int i=0;i<8;i++) sdf.pack.sens[i]=stab[i];
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
    if(rcf.pack.con[0]==0x12 && rcf.pack.did==termID)  // received ACK_ID frame for this node  
      {
      idreq_ack=1; gwID=rcf.pack.sid; 
      }
    if(rcf.pack.con[0]==0x14 && rcf.pack.did==termID)  // received DATACK frame for this node  
      {
      dtsnd_ack=1; gwID=rcf.pack.sid; timeout=rcf.pack.tout;
      } 
    Serial.printf("Term:%08X, ACK from Gateway:%08X\n",(uint32_t)rcf.pack.did,(uint32_t)gwID);
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
