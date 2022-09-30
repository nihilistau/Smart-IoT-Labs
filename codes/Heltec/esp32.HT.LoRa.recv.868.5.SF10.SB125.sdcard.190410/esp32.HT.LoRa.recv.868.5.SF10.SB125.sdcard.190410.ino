
#include <WiFi.h>
#include <Wire.h>
#include "esp_wpa2.h"
#include <SPI.h>
#include <LoRa.h>
#include <SD.h>
#include "RTClib.h"       //to show time
#include "ThingSpeak.h"

char ssid[] = "PhoneAP";          //  your network SSID (name) 
char pass[] = "smartcomputerlab";   // your network passw
// to be replaced by eduroam ..
const char* edussid = "eduroam"; // your ssid
const char* edupass = ""; // your ssid
#define EAP_ID "bakowski-p"
#define EAP_USERNAME "bakowski-p" //removed for obvious reasons
#define EAP_PASSWORD "Sydney2010"
int status = WL_IDLE_STATUS;


#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define SD_CS 23
#define SD_SCK 17
#define SD_MOSI 12
#define SD_MISO 13

#define LOG_PATH "/lora_recv.log"

SPIClass sd_spi(HSPI);



#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, reset

#include "introd.h"

RTC_DS3231 rtc;

unsigned long myChannelNumber[8] = {1,4,17,0,0,0,0,0 };  // IoTDevKit1, IoTDevKit2, IoTDevKit3
const char * myWriteAPIKey[8] = { "HEU64K3PGNWG36C4","4M9QG56R7VGG8ONT","MEH7A0FHAMNWJE8P",NULL,NULL,NULL,NULL,NULL};
//  channel names IoTDevKit1, IoTDevKit2, IoTDevKit3


WiFiClient  client;

int rssi;

int rdcount=0;

union tspack
  {
    uint8_t frame[24];
    struct packet
      {
        uint8_t head[4];  // head[3] is length for sensors or text, text takes 1 on MSbit
        uint32_t pnum;
        float sensor[4];
      } pack;
  } rdf;  // data frame 

int sf=10,sb=125E3; 
long freq=868500E3; 
//long freq=434500E3; 


void dispLabel()
{
  u8x8.clear();
  u8x8.draw2x2String(0,0,"- LoRa -");
  delay(1000);
  u8x8.draw2x2String(0,2,"receiver");
  delay(1000);
  u8x8.draw2x2String(0,4,"Polytech");
  delay(1000);
  u8x8.draw2x2String(0,6,"- LS2N -");
  delay(3000);
  u8x8.clear();
}


void dispPar()
{
  char dbuf[32];
  u8x8.clear();;
  u8x8.draw2x2String(0,0,"868.5MHz");
  delay(1000);
  u8x8.draw2x2String(0,2," SF=10  ");
  delay(1000);
  u8x8.draw2x2String(0,4,"125 KHz ");
  delay(1000);
  u8x8.draw2x2String(0,6,"Cyc=20s ");
  delay(3000);
  u8x8.clear();
}


void dispData()  
{
  char dbuf[32];
  u8x8.clear();
  memset(dbuf,0x00,32);
  sprintf(dbuf,"Temp:%2.2f",rdf.pack.sensor[0]);
  u8x8.drawString(0, 1,dbuf);
  memset(dbuf,0x00,32);
  sprintf(dbuf,"Humi:%2.2f",rdf.pack.sensor[1]);
  u8x8.drawString(0, 2,dbuf);
  memset(dbuf,0x00,32);
  sprintf(dbuf,"Lumi:%5.5d",(int)rdf.pack.sensor[2]);
  u8x8.drawString(0, 3,dbuf);
  memset(dbuf,0x00,32);
  sprintf(dbuf,"Pnum:%5.5d",(int)rdf.pack.sensor[3]);
  u8x8.drawString(0, 4,dbuf);
  memset(dbuf,0x00,32); 
  sprintf(dbuf,"RSSI%3.3d", LoRa.packetRssi());
  u8x8.draw2x2String(0, 6,dbuf);

}

int api=0, constep=0, apcon=0;
 
void setup() {
  Serial.begin(9600);
  Wire.begin(21,22);
  pinMode(buttonPin, INPUT_PULLUP);
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);

    dispLabel();




  
  Serial.println(sf);
  Serial.println(sb);
  Serial.println(freq);
  
  pinMode(26, INPUT);  // recv interrupt
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sb);
  LoRa.setCodingRate4(8);
  LoRa.enableCrc();

  dispPar();

    if (!rtc.begin()) {
Serial.println("Couldn't find RTC");
while (1);
}

rtc.adjust(DateTime(__DATE__, __TIME__));

  api=setAP();


    // SD Card
    sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    if (!SD.begin(SD_CS, sd_spi))
        Serial.println("SD Card: mounting failed.");
    else 
Serial.println("SD Card: mounted.");

   WiFi.disconnect(true);
   delay(1000);
   if(api==2)
     { 
      WiFi.mode(WIFI_STA);
      WiFi.disconnect(true);
      esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ID, strlen(EAP_ID));
      esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_USERNAME, strlen(EAP_USERNAME));
      esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
      esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
      esp_wifi_sta_wpa2_ent_enable(&config);
      WiFi.begin(ssid);
      Serial.println("Waiting for Eduroam WiFi");
      constep=0; 
     while (WiFi.status() != WL_CONNECTED) {
       delay(500);
       Serial.print(".");
       constep++; if(constep==100) break;
       }
       if(constep==100) 
         {
          Serial.println("Eduroam WiFi fails !!!");
          u8x8.clear();
          u8x8.drawString(0,2,"WiFi arror");
         }
       else 
         {
          apcon=1;
          Serial.println("Eduroam WiFi connected");
          Serial.println(WiFi.localIP());
          ThingSpeak.begin(client);
          delay(1000);
          u8x8.clear();
          u8x8.drawString(0,2,"WiFi OK");
          Serial.println("ThingSpeak begin");
          delay(1000);
         }
     }
     
    if(api==3)
    {
    WiFi.begin(ssid, pass); 
    delay(1000);    
    constep=0; 
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      constep++; if(constep==100) break;
      }
    if(constep==100) 
      { 
      Serial.println("PhoneAP WiFi fails !!!");
      u8x8.clear();
      u8x8.drawString(0,2,"WiFi arror"); 
      }
    else 
      {
      apcon=1;
      Serial.println("PhoneAP WiFi connected");
      u8x8.clear();
      u8x8.drawString(0,2,"WiFi OK");
      ThingSpeak.begin(client);
      delay(1000);
      Serial.println("ThingSpeak begin");
      delay(1000);
      }
    }
}


int i=0;

//char *lora_buf="test LoRa";
char lora_buf[128]; 

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize==24)
    {
    Serial.println("packet");
    i=0;
    while (LoRa.available()) 
      {
      rdf.frame[i]=LoRa.read();i++;
      }
    rssi=LoRa.packetRssi();
    DateTime now = rtc.now();
    Serial.println(now.second(), DEC);
    Serial.println(now.minute(), DEC);
    Serial.println(now.hour(), DEC);
    Serial.println(rdf.pack.head[2]);
    Serial.println(rdf.pack.sensor[0]);
    Serial.println(rdf.pack.sensor[1]);
    Serial.println(rdf.pack.sensor[2]);
    Serial.println(rdf.pack.sensor[3]);
    Serial.println();
    if(apcon)
      {
      ThingSpeak.setField(1, rdf.pack.sensor[0]);
      ThingSpeak.setField(2, rdf.pack.sensor[1]);
      ThingSpeak.setField(3, rdf.pack.sensor[2]);
      ThingSpeak.setField(4, (float)rssi);
      while (WiFi.status() != WL_CONNECTED) 
        {
        delay(500);
        Serial.print(".");
        }
      ThingSpeak.writeFields(4, "4M9QG56R7VGG8ONT");
      }  
    dispData();   
    File test = SD.open(LOG_PATH, FILE_APPEND);
        if (!test) 
          {
          Serial.println("SD Card: writing file failed.");
          } 
        else 
          {
          Serial.printf("SD Card: appending data to %s.\n", LOG_PATH);
          memset(lora_buf,0x00,128);
          sprintf(lora_buf,"%2.2f,%2.2f,%2.2f,%d",rdf.pack.sensor[0],rdf.pack.sensor[1],rdf.pack.sensor[2],rssi);
          test.write((uint8_t *)lora_buf, strlen(lora_buf));
          test.printf("\n");
          test.close();
          }
    delay(10000);  // ending the reception cycle
   }  
   // in the loop waiting for frame     
}

