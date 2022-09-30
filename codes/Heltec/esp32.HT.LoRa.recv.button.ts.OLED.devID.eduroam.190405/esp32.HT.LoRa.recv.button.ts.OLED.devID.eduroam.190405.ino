
#include <WiFi.h>
#include "esp_wpa2.h"
#include "esp_deep_sleep.h"
#include <SPI.h>
#include <LoRa.h>
#include "ThingSpeak.h"
// sensitivity table
int senst[5][7] = { // 6     7    8    9   10   11   12   - spreading factor
                    {-125,-129,-133,-136,-138,-142,-143}, //  31250 Hz
                    {-121,-126,-129,-132,-135,-138,-140}, //  62500 Hz
                    {-118,-123,-126,-129,-132,-135,-137}, // 125000 Hz
                    {-115,-120,-123,-126,-129,-132,-134}, // 250000 Hz
                    {-112,-117,-120,-123,-126,-129,-131}, // 500000 Hz
};

//char ssid[] = "Livebox-08B0";          //  your network SSID (name) 
//char pass[] = "G79ji6dtEptVTPWmZP";   // your network passw
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

#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, reset
#include "introd.h"  // configuration function

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


void dispLabel()
{
  u8x8.clear();
  u8x8.draw2x2String(0,0,"Polytech");
  delay(1000);
  u8x8.draw2x2String(0,2,"Facultes");
  delay(1000);
  u8x8.draw2x2String(4,4,"LoRa");
  delay(1000);
  u8x8.draw2x2String(5,6,"link");
  delay(3000);
  u8x8.clear();
}


void dispDatarate(int dr,int sf, int sb)
{
  char dbuf[16];int sbind=0;int sensitivity=0;
  u8x8.clear();
  sprintf(dbuf,"DR=%5.5d bps",dr);
  u8x8.drawString(0,1,dbuf);
  delay(1000);
  u8x8.drawString(0,2,"CR=4/8");
  delay(1000);
  u8x8.drawString(0,3,"CRC check set");
  switch (sb)
    {
    case 31250: sbind=0;break;
    case 62500: sbind=1;break;
    case 125000: sbind=2;break;
    case 250000: sbind=3;break;
    case 500000: sbind=4;break;
    default: sbind=0;break;
    }

  Serial.println(sf-6);Serial.println(sbind);
  delay(1000);  
  sensitivity=senst[sbind][sf-6];
  sprintf(dbuf,"Sens=%3.3d dBm",sensitivity);
  u8x8.drawString(0,4,dbuf);
  delay(1000);  
  u8x8.drawString(0,6,"Packet size 24B");
  delay(3000);
  u8x8.clear();
}

int bitrate(int psf, int psb)  // by default cr=4
{
  int res;
  res= (int) ((float)psf*(float)psb/(2.0*pow(2,psf)));
  return res;
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
  sprintf(dbuf,"RSSI%3.3d", LoRa.packetRssi());
  u8x8.drawString(0, 5,dbuf);

}

int api=0, constep=0, apcon=0;
 
void setup() {
  int sf,sb; long freq; int br=0;
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  dispLabel();
  introd(&sf,&sb,&freq);

  br=bitrate(sf,sb);
  dispDatarate(br,sf,sb);
  
  Serial.println(sf);
  Serial.println(sb);
  Serial.println(freq);
  Serial.println(br);
  
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

  api=setAP();

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

void loop() {
  char dbuff[32]; 
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize==24) {
    Serial.println("packet");
    i=0;
    while (LoRa.available()) 
    {
      rdf.frame[i]=LoRa.read();i++;
    }
    Serial.println(rdf.pack.head[2]);
    Serial.println(rdf.pack.sensor[0]);
    Serial.println(rdf.pack.sensor[1]);
    Serial.println(rdf.pack.sensor[2]);
    Serial.println(rdf.pack.sensor[3]);
    Serial.println();
//    if(rdf.pack.head[2] & 0x80) ThingSpeak.setField(1, rdf.pack.tsdata.sensor[0]);
//    if(rdf.pack.head[2] & 0x40) ThingSpeak.setField(2, rdf.pack.tsdata.sensor[1]);
//    if(rdf.pack.head[2] & 0x20) ThingSpeak.setField(3, rdf.pack.tsdata.sensor[2]);
//    if(rdf.pack.head[2] & 0x10) ThingSpeak.setField(4, rdf.pack.tsdata.sensor[3]);

// for test without sender
  if(apcon)
    {
    ThingSpeak.setField(1, rdf.pack.sensor[0]);
    ThingSpeak.setField(2, rdf.pack.sensor[1]);
    ThingSpeak.setField(3, rdf.pack.sensor[2]);
    ThingSpeak.setField(4, (float)LoRa.packetRssi());

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      }
    ThingSpeak.writeFields(4, "4M9QG56R7VGG8ONT");
    }  
    dispData();
    delay(10000);
  }       
  
}
