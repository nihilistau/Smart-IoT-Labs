
#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <U8x8lib.h>
#include <Wire.h>
#include "SparkFunHTU21D.h"
#include <TinyGPS.h>

// Pin definition of WIFI LoRa 32
// HelTec AutoMation 2017 support@heltec.cn 
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define freq    868E6  //you can set band here directly,e.g. 868E6,915E6
#define sf 8
#define sb 125E3       // set the signal bandwidth; 125KHz,[250KHz,500Kz]
HTU21D TempHumi;
TinyGPS gps;
HardwareSerial ss(2); 
// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15,4,16);  // clock,data,reset
int i=0,rssi;

union tspack
  {
    uint8_t frame[48];
    struct packet
      {
        uint8_t head[4];
        uint16_t num;
        uint16_t tout;
        float sensor[4];
      } pack;
  } sdf,sbf,rdf,rbf;  // data frame and beacon frame

  int sdcount=0, sbcount=0, rdcount=0, rbcount=0;
  int sr=1;

void dispData(int sr)  // sr=1 - send, sr=0 - receive
{
  char dbuf[32];
  u8x8.clear();
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"SPacket:%4.4d",sdcount);
  else sprintf(dbuf,"RPacket:%4.4d",rdcount);
  u8x8.drawString(0, 0,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Temp:%2.2f",sdf.pack.sensor[2]);
  else sprintf(dbuf,"Temp:%2.2f",rdf.pack.sensor[2]);
  u8x8.drawString(0, 2,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Humi:%2.2f",sdf.pack.sensor[3]);
  else sprintf(dbuf,"Humi:%2.2f",rdf.pack.sensor[3]);
  u8x8.drawString(0, 3,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Lat:%f",sdf.pack.sensor[5]);
  else sprintf(dbuf,"Lat:%f",rdf.pack.sensor[5]);
  u8x8.drawString(0, 4,dbuf);
  memset(dbuf,0x00,32);
  if(sr) sprintf(dbuf,"Lon:%f",sdf.pack.sensor[6]);
  else sprintf(dbuf,"Lon:%f",rdf.pack.sensor[6]);
  u8x8.drawString(0, 5,dbuf);
  memset(dbuf,0x00,32);
  if(!sr) sprintf(dbuf,"RSSI:%4.4d",rssi);
  else sprintf(dbuf,"                ");
  u8x8.drawString(0, 7,dbuf);
  delay(3000);
}

void dispGPS(int sat,float flat,float flon, char *date, char *time)
{
  char dbuf[16];
  u8x8.clear();
  Serial.println("GPS coordinates");
  u8x8.drawString(0, 1, "GPS coordinates");
  sprintf(dbuf,"SatNum:%d",sat);
  u8x8.drawString(0, 2, dbuf);
  sprintf(dbuf,"Lat:%f",flat);
  u8x8.drawString(0, 3,dbuf);
  sprintf(dbuf,"Long:%f",flon);
  u8x8.drawString(0, 4, dbuf);
  u8x8.drawString(0, 6, date);
  u8x8.drawString(0, 7, time);
  delay(3000);
}

void dispPara()
{
  char dbuf[16];
  Serial.println("LoRa Sender");
  u8x8.drawString(0, 1, "LoRa Sender");
  sprintf(dbuf,"Freq:%3.2f MHz",(float)freq/1000000.0);
  u8x8.drawString(0, 2, dbuf);
  LoRa.setSpreadingFactor(sf);
  sprintf(dbuf,"SF:%d",sf);
  u8x8.drawString(0, 3, dbuf);
  LoRa.setSignalBandwidth(sb);
  sprintf(dbuf,"SB:%3.2f KHz",(float)sb/1000.0);
  u8x8.drawString(0, 4, dbuf);
  delay(6000);
  u8x8.clear();
}



void sendGPSData(float slat, float slon) {
  LoRa.beginPacket();                   // start packet
  sdf.pack.head[0]=0xff;                 // set destination address
  sdf.pack.head[1]=0xff;                 // set sender address
  sdf.pack.head[2]=0x36;                 // 00110110 set data type: sensors
  sdf.pack.head[3]=0x10;                 // set payload length: 16 bytes
  sdf.pack.num=(uint16_t) sdcount;        // set data packet count
  sdf.pack.tout=(uint16_t) 0;            // set timeout

  // add GPS data
  sdf.pack.sensor[2] = TempHumi.readTemperature();  // temperature
  sdf.pack.sensor[3] = TempHumi.readHumidity();    // humidity    
  sdf.pack.sensor[5] = slat;                       // latitude
  sdf.pack.sensor[6] = slon;                       // longitude                       
  LoRa.write(sdf.frame,40);              // load long data frame
  LoRa.endPacket();                     // finish packet and send it
  dispData(1);
  sdcount++;                           // increment message ID
}



void setup() {
  Serial.begin(9600);                   // initialize serial
  pinMode(26, INPUT);  // recv interrupt
  ss.begin(9600, SERIAL_8N1, 17, 16);
  Serial.println("start GPS");
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  Serial.println("LoRa Duplex");

  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }
  dispPara();
  Serial.println("LoRa init succeeded.");
  Wire.begin(); 
  TempHumi.begin();
  Serial.println("HTU21D Example!");

}


void loop() {
bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;
  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 20000;)
  {
    while (ss.available())
    {
      char c = ss.read();
      //Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    float flat, flon;
    unsigned long age;
    unsigned long ldate,ltime,tage;
    char sdate[24],stime[24],sstime[24],gpsbuff[64];

    gps.f_get_position(&flat, &flon, &age);
    gps.get_datetime(&ldate,&ltime,&tage);
    sprintf(sdate,"Date:%ld",ldate); //Serial.println(sdate);
    sprintf(stime,"Time:%ld",ltime); //Serial.println(sdate);
    Serial.println(sdate);
    Serial.println(stime);
    memset(sstime,0x00,24);
    if(strlen(stime)==8)
        strncpy(sstime,stime,11);
    else { strncpy(sstime,"Time:0",6); strncat(sstime,stime+5,5); }    
    Serial.println(sstime);  
    memset(gpsbuff,0x00,64);
    sprintf(gpsbuff,"Sat:%d  Lat:%2.6f Long:%2.6f", gps.satellites(),flat,flon);
    Serial.println(gpsbuff);
    dispGPS(gps.satellites(),flat,flon,sdate,sstime);  
    sendGPSData(flat,flon); 
     
  }
}



