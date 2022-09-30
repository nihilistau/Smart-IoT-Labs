
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Sodaq_SHT2x.h>
#include <BH1750.h>

BH1750 lightMeter;
// sensitivity table
int senst[5][7] = { // 6     7    8    9   10   11   12   - spreading factor
                    {-125,-129,-133,-136,-138,-142,-143}, //  31250 Hz
                    {-121,-126,-129,-132,-135,-138,-140}, //  62500 Hz
                    {-118,-123,-126,-129,-132,-135,-137}, // 125000 Hz
                    {-115,-120,-123,-126,-129,-132,-134}, // 250000 Hz
                    {-112,-117,-120,-123,-126,-129,-131}, // 500000 Hz
};


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

uint8_t devID;

int sdcount=0;

union tspack
  {
    uint8_t frame[24];
    struct packet
      {
        uint8_t head[4];  // head[3] is length for sensors or text, text takes 1 on MSbit
        uint32_t pnum;
        float sensor[4];
      } pack;
  } sdf;  // data frame 


 union {
   struct 
     {
     int rssi;
     uint8_t dev;
     } pack;
   uint8_t buff_rssi[5];
} rurssi,wurssi;

void dispLabel()
{
  u8x8.clear();
  u8x8.draw2x2String(0,0,"- LoRa -");
  delay(1000);
  u8x8.draw2x2String(0,2,"Polytech");
  delay(1000);
  u8x8.draw2x2String(0,4,"Bat LS2N");
  delay(1000);
  u8x8.draw2x2String(0,6,"VLR link");
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
  sprintf(dbuf,"Temp:%2.2f",sdf.pack.sensor[0]);
  u8x8.drawString(0, 1,dbuf);
  memset(dbuf,0x00,32);
  sprintf(dbuf,"Humi:%2.2f",sdf.pack.sensor[1]);
  u8x8.drawString(0, 2,dbuf);
  memset(dbuf,0x00,32);
  sprintf(dbuf,"Lumi:%5.5d",(int)sdf.pack.sensor[2]);
  u8x8.drawString(0, 3,dbuf);
  memset(dbuf,0x00,32);  sdcount++;
  sprintf(dbuf,"Pnum:%6.6d",sdcount);
  u8x8.drawString(0, 5,dbuf);

}

int cyc=10;

void setup() {
  int sf,sb; long freq; int br=0;

  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  dispLabel();
  introd(0,&devID,&sf,&sb,&freq);  // pin-0 (PRG on board), 17 - Wemos shield

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
  //LoRa.setTxPower(14);
  LoRa.setCodingRate4(8);
  LoRa.enableCrc();

  cyc=setCycle();
  
 Wire.begin(21,22); 
 lightMeter.begin(); 
 delay(1000);
}



void loop() {
  LoRa.beginPacket();
  sdf.pack.head[0]= 0xff;  // destination term - broadcast
  sdf.pack.head[1]= devID; // source term 3
  sdf.pack.head[2]= 0xf0;  // field mask - filed1,field2, field3
  sdf.pack.head[3]= 0x10;  // data size 16 bytes
  sdf.pack.pnum= (uint16_t) sdcount;
  sdf.pack.sensor[2] = (float)lightMeter.readLightLevel();
  sdf.pack.sensor[0] = SHT2x.GetTemperature();
  sdf.pack.sensor[1] = SHT2x.GetHumidity();
  sdf.pack.sensor[3] = (float) sdcount;
  LoRa.write(sdf.frame,24);
  LoRa.endPacket();
  Serial.print("Temp:");Serial.println(sdf.pack.sensor[0]);
  Serial.print("Humi:");Serial.println(sdf.pack.sensor[1]);
  Serial.print("Lumi:");Serial.println(sdf.pack.sensor[2]);
  dispData();
  delay(1000*cyc+random(10000+5000*(int)devID));
}
