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

#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock,

int buzzPin = 17, val;
unsigned long start;
int count=0;

union 
  {
    uint8_t frame[28];
    struct 
      {
        uint8_t head[4];
        char mess[24];
      } data;
  } pack;


void setup() {
  // put your setup code here, to run once:
pinMode(17,OUTPUT);  
Serial.begin(9600);
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.clear();
Serial.println();
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("Starting LoRa OK!");
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sb);

  u8x8.drawString(0,0,(char*)"Starting !");

}

int i=0;
void loop() {
  char dbuff[32];
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {

    // received a packet
    Serial.print("Received packet '");
    i=0;
    while (LoRa.available()) 
    {
      pack.frame[i]=LoRa.read();i++;
    }
    
    u8x8.clear();
    sprintf(dbuff,"Term: %2.2d",pack.data.head[1]);
    u8x8.drawString(0,0,dbuff);
    sprintf(dbuff,"Mess: %s",pack.data.mess);
    u8x8.drawString(0,1,dbuff);
    
    digitalWrite(17,HIGH);
    delay(1000);
    digitalWrite(17,LOW);
    
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
