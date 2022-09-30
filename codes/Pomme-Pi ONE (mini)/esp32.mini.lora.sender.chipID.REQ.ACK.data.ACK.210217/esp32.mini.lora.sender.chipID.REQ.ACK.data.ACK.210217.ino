//Attention low upload speed- 460800

#include <LoRa.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"

// Initialize the OLED display using Arduino Wire:
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL

uint64_t chipid;  
uint32_t sendID;
uint32_t recvID;

// with LoRa modem RFM95 and green RFM board - int and RST to solder

#define SS       5 // 26     // D0 - to NSS
#define RST     15  //16     // D4  - RST
#define DI0     26      // D8 - INTR

#define SCK     18      // D5 - CLK
#define MISO    19      // D6 - MISO
#define MOSI    23      // D7 - MOSI
#define BAND    434E6

int sf=7;
long sbw=125E3;

uint16_t timeout;

union 
{
  uint8_t frame[32];
  struct
  {
    uint32_t did;       // destination identifier chipID (4 lower bytes)
    uint32_t sid;       // source identifier chipID (4 lower bytes)
    uint8_t  con[2];    // control field
    char     pass[16];  // password - 16 characters
    uint16_t tout;      // timeout
    uint8_t  pad[4];    // future use
  } pack;               // control packet
} scf,rcf;              // send control frame , receive control frame

int ack=0;

union 
{
  uint8_t frame[64];    // MQTT frame to publish on the given topic
  struct
  {
    uint32_t did;       // destination identifier chipID (4 lower bytes)
    uint32_t sid;       // source identifier chipID (4 lower bytes)
    uint8_t  con[2];    // control field
    char     topic[24];  // topic name - e.g. /esp32/Term1/Sens1
    char     mess[24];   // message value
    uint8_t  pad[6]; // future use
  } pack;               // data packet
} sdf,rdf;              // send data frame , receive data frame

void onReceive(int packetSize) {
  if(packetSize==32)
    {
    for (int i = 0; i < packetSize; i++) 
    rcf.frame[i]= LoRa.read();
    Serial.printf("%08X\n",(uint32_t)rcf.pack.sid);
    Serial.printf("%X,%X\n",rcf.pack.con[0],rcf.pack.con[1]);
    timeout= rcf.pack.tout;
  }
  if(rcf.pack.con[0]==0x01) 
    {
      recvID = rcf.pack.sid; ack=1;
    }
  // print RSSI of packet
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());
}


void setup() {
  Serial.begin(9600);
  chipid=ESP.getEfuseMac();
  sendID=(uint32_t)chipid;
  
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);

  Serial.begin(9600);
  delay(1000);
  Serial.println();Serial.println();
  Serial.println("Starting LoRa Sender");
  Serial.printf("%08X\n",(uint32_t)chipid);
  // Initialising the UI will init the display too.

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  else 
   {
   Serial.println("Starting LoRa ok!");
   LoRa.setSpreadingFactor(sf);
   LoRa.setSignalBandwidth(sbw);

  // register the receive callback
  LoRa.onReceive(onReceive);
      LoRa.receive(); 
   }
}

int counter=0;
char buff[32];

void send_REQ()
{
  Serial.println("Sending AC REQ packet: ");
  // send packet
  LoRa.beginPacket();
  scf.pack.did=(uint32_t)0;
  scf.pack.sid=(uint32_t)sendID;
  scf.pack.con[0]=0x00; scf.pack.con[1]=0x01;       // ID request
  strncpy(scf.pack.pass,"passwordpassword",16);   // 16 character password
  LoRa.write(scf.frame,32);
  LoRa.endPacket();
}

void publish(char *topic, char *mess)
{
  Serial.println("Sending Publish packet: ");
  // send packet
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)recvID;    // receiver (GW) ID
  sdf.pack.sid=(uint32_t)sendID;
  sdf.pack.con[0]=0x00; sdf.pack.con[1]=0x10;       // publish data
  strcpy(sdf.pack.topic,topic);   // topic name
  strcpy(sdf.pack.mess,mess);   // topic name
  LoRa.write(sdf.frame,64);
  LoRa.endPacket();
}

float temper= 23.45;

void loop() {
  if(ack==0) 
    { 
    send_REQ();  Serial.println("Set recv mode");
    LoRa.receive(); 
    }
  else
    {
    char topic[24],message[24];
    strcpy(topic,"/esp32/Term1/Sens1");
    sprintf(message,"Temp=%f",temper); temper=temper+0.1;
    publish(topic,message);  Serial.println("Message published"); 
    LoRa.receive();  
    }
  if(!timeout) delay(5000);
  else 
    {
      Serial.print("Going to deep_sleep mode for: ");Serial.print(100*timeout);
      Serial.println("ms");
      esp_sleep_enable_timer_wakeup(1000*100*timeout);
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); delay(100);
      esp_deep_sleep_start();
      //delay(100*timeout);
    }

}
