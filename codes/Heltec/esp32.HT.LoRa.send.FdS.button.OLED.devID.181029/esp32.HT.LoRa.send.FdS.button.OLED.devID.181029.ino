
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "SHT21.h"

SHT21 SHT21;

#define Addr 0x4A

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

int rssi;

int sdcount=0, rdcount=0;

union tspack
  {
    uint8_t frame[128];
    struct packet
      {
        uint8_t head[4];  // head[3] is length for sensors or text, text takes 1 on MSbit
        uint16_t num;
        uint16_t tout;
        union {
              float sensor[8];
              char text[32];
              } tsdata;
      } pack;
  } sdf,sbf,rdf,rbf;  // data frame and beacon frame


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
  u8x8.drawString(0,0,"Polytech");
  delay(1000);
  u8x8.drawString(0,2,"Faculte-Sciences");
  delay(1000);
  u8x8.drawString(0,4,"Very Long Range");
  delay(1000);
  u8x8.drawString(2,6,"LoRa link");
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

void dispData(int sr, int rssi, uint8_t dev)  // sr=1 - send, sr=0 - receive
{
  char dbuf[32];
  u8x8.clear();
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"SPacket:%5.5d",sdcount);
  else sprintf(dbuf,"RPacket:%5.5d",rdcount);
  u8x8.drawString(0, 0,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"SDevice:%5.5d",(int)sdf.pack.head[1]);
  else sprintf(dbuf,"RDevice:%5.5d",(int)rdf.pack.head[1]);
  u8x8.drawString(0, 1,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Temp:%2.2f",sdf.pack.tsdata.sensor[0]);
  else sprintf(dbuf,"Temp:%2.2f",rdf.pack.tsdata.sensor[0]);
  u8x8.drawString(0, 3,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Humi:%2.2f",sdf.pack.tsdata.sensor[1]);
  else sprintf(dbuf,"Humi:%2.2f",rdf.pack.tsdata.sensor[1]);
  u8x8.drawString(0, 4,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Lumi:%5.5d",(int)sdf.pack.tsdata.sensor[2]);
  else sprintf(dbuf,"Lumi:%5.5d",(int)rdf.pack.tsdata.sensor[2]);
  u8x8.drawString(0, 5,dbuf);
  memset(dbuf,0x00,32);
  if(!sr) sprintf(dbuf,"Srssi:%3.3d/%2.2d",(int)rdf.pack.tsdata.sensor[3],(int)rdf.pack.tout);
  u8x8.drawString(0, 6,dbuf);
  memset(dbuf,0x00,32);
  if(!sr) sprintf(dbuf,"Rrssi:%3.3d/%2.2d",rssi,dev);
  else sprintf(dbuf,"                ");
  u8x8.drawString(0, 7,dbuf);
  if(!sr) delay(3000);
}


void LoRaSendTask( void * pvParameters )
{
int i=0;
unsigned int data[2];
int exponent,mantissa;
float luminance;
while(1)
  {  

Wire.beginTransmission(Addr);
Wire.write(0x03);
Wire.endTransmission();
 
// Request 2 bytes of data
Wire.requestFrom(Addr, 2);
 
// Read 2 bytes of data luminance msb, luminance lsb
if (Wire.available() == 2)
{
data[0] = Wire.read();
data[1] = Wire.read();
}
 
// Convert the data to lux
exponent = (data[0]&0xF0)>>4;
mantissa = ((data[0]&0x0F) << 4) | (data[1]&0x0F);
luminance = pow(2, exponent) * mantissa * 0.045;  
  LoRa.beginPacket();
  sdf.pack.head[0]= 0xff;  // destination term - broadcast
  sdf.pack.head[1]= devID; // source term 3
  sdf.pack.head[2]= 0xf0;  // field mask - filed1,field2, field3
  sdf.pack.head[3]= 0x10;  // data size 16 bytes
  sdf.pack.num= (uint16_t) sdcount;
  sdf.pack.tout= (uint16_t) 0; // timeout 0 - here dev of rssi

  sdf.pack.tsdata.sensor[2] = luminance;
  sdf.pack.tsdata.sensor[0] = SHT21.getTemperature();
  sdf.pack.tsdata.sensor[1] = SHT21.getHumidity();

  LoRa.write(sdf.frame,24);
  LoRa.endPacket();
  Serial.print("Temp:");Serial.println(sdf.pack.tsdata.sensor[0]);
  Serial.print("Humi:");Serial.println(sdf.pack.tsdata.sensor[1]);
  Serial.print("Lumi:");Serial.println(sdf.pack.tsdata.sensor[2]);
  if(sdcount<64000) sdcount++; else sdcount=0;
  dispData(1,0,0);                 
  vTaskDelay(10000+random(10000+5000*(int)devID));
  }
}


void setup() {
  int sf,sb; long freq; int br=0;
  Serial.begin(9600);
  Wire.begin(21,22);
  Wire.beginTransmission(Addr);
  Wire.write(0x02);
  Wire.write(0x40);
  Wire.endTransmission();
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  dispLabel();
  introd(&sf,&sb,&freq);  // set LoRa parameters
  br=bitrate(sf,sb);      // calculate data rate
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
  LoRa.setCodingRate4(8);  // set 4/8 CR
  LoRa.enableCrc();
  //LoRa.setTxPower(20);
  
xTaskCreate(
                    LoRaSendTask,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL);
 
  Serial.println("LoRaSendTask created...");
  delay(3000);
}



void loop() {
    delay(16000);
}
