#include <SPI.h>
#include <LoRa.h>

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define freq    434E6  //you can set band here directly,e.g. 868E6,915E6
int  sf=8;
int sb=125E3;       // set the signal bandwidth; 125KHz,[250KHz,500Kz]
uint8_t sw=0xF3;

#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock,

int radPin = 17, val;
int relayPin = 21;
unsigned long start;

char dbuff[32];
int mc=0;
int count=0;

union 
  {
    uint8_t frame[8];
    struct 
      {
        uint8_t head[4];
        uint32_t numb;
      } data;
  } pack;


void setup() {
  // put your setup code here, to run once:
pinMode(17,INPUT);  
pinMode(22,OUTPUT);  
Serial.begin(9600);
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.clear();
Serial.println();
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(434E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Starting LoRa OK!");
  u8x8.drawString(0,0,"LoRa OK");
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sb);
  LoRa.setSyncWord(sw);
}

int pnum=0;

void loop() {
char dbuf[32];
LoRa.setFrequency(868E6);
delay(20);
val = digitalRead(radPin);
delay(20);
LoRa.setFrequency(434E6);
delay(20);
LoRa.setFrequency(434E6);
if (val == LOW)
{
  count=0;
}
else
{
  count=1; pnum++;
      LoRa.beginPacket();
      pack.data.head[0]=0xff;pack.data.head[1]=0x01;  // dest, source
      pack.data.head[2]=0x01;pack.data.head[3]=0x04;  // type, length
      pack.data.numb= (uint32_t)pnum;
      LoRa.write(pack.frame,pack.data.head[3]+4);
      LoRa.endPacket();
      sprintf(dbuf,"DevID:%2.2d",(int)pack.data.head[1]);
      u8x8.drawString(0,3,dbuf);
      sprintf(dbuf,"Pnum:%6.6d",pnum);
      u8x8.drawString(0,5,dbuf);
      digitalWrite(22,HIGH);
      delay(3000);
      digitalWrite(22,LOW);
}
delay(20);
}
