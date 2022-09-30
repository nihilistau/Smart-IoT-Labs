#include <SPI.h>
#include <LoRa.h>

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

int  sf=8;
int sb=125E3;       // set the signal bandwidth; 125KHz,[250KHz,500Kz]

#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, reset

QueueHandle_t  dqueue;  // queues for beacons and data frames
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


void dispTag(char *rtag)  // sr=1 - send, sr=0 - receive
{
  char dbuf[32];
  u8x8.clear();
  memset(dbuf,0x00,32);
  u8x8.drawString(0, 1,"Received tag");
  u8x8.drawString(0, 2,rtag);
}

void dispName(char *rtag)  // sr=1 - send, sr=0 - receive
{
  char dbuf[32];
  u8x8.clear();
  memset(dbuf,0x00,32);
  u8x8.drawString(0, 1,"Received name");
  u8x8.drawString(0, 3,rtag);
  u8x8.drawString(0, 4,rtag+16);
  delay(2000);
}

void onReceive(int packetSize) 
{
int i=0; 
uint8_t rbuf[128];
i=0;
  if (packetSize==0) return;   // if there's no packet, return
  if (packetSize!=40) return;   // if there's no packet, return
  else
    {
    while (LoRa.available()) {
      rdf.frame[i]=LoRa.read();i++;
      }
    Serial.println(rdf.pack.head[0]);
     // if(rdbuff[0]!=0x02) return;
    Serial.println(rdf.pack.head[1]);
    Serial.println(rdf.pack.head[2]);
    Serial.println(rdf.pack.head[3]); 
    Serial.println(rdf.pack.num); 
    Serial.println(rdf.pack.tsdata.text); 
    Serial.println(rdf.pack.tsdata.text+16); 
    dispName(rdf.pack.tsdata.text);
//    if(rdcount<64000) rdcount++; else rdcount=0;
//    xQueueReset(dqueue); // to keep only the last element
//    xQueueSend(dqueue, rbbuff, portMAX_DELAY); 
    }
    //LoRa.receive();
}

void setup() {
  int sf,sb; long freq; int br=0;
  Serial.begin(9600);
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  //dqueue = xQueueCreate(4, 8); // queue for 4 LoRaTS data frames

  pinMode(26, INPUT);  // recv interrupt
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(870E6)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }
  LoRa.setCodingRate4(8);
  LoRa.enableCrc();
//  LoRa.onReceive(onReceive);
//  LoRa.receive();  
}

int i=0;
void sendDataPacket()
{
  LoRa.beginPacket();
  memset(sdf.frame,0x00,24);
  sdf.pack.head[0]= 0x01;  // destination term 1
  sdf.pack.head[1]= 0x02;  // source term 2
  sdf.pack.head[2]= 0x80;  // field mask - filed1 set
  sdf.pack.head[3]= 0x90;  // data size 16 bytes in text - RFID tag - MSB set
  sdf.pack.num= (uint16_t) sdcount;
  sdf.pack.num= (uint16_t) 0; // timeout 0
  if(i%2) strncpy(sdf.pack.tsdata.text,"0E F6 BE 23",12);  // "0E F6 BE 23"
  else strncpy(sdf.pack.tsdata.text,"1E F6 BE 23",12);
  delay(200);i++;
  Serial.println(sdf.pack.tsdata.text);
  dispTag(sdf.pack.tsdata.text);
  LoRa.write(sdf.frame,24);
  LoRa.endPacket();
}

long lastSendTime = 0;        // last send time
int interval = 2000; // interval between sends

void loop() 
{
if(millis()-lastSendTime>interval) 
  {
  sendDataPacket();
  lastSendTime = millis();            // timestamp the message
  interval = random(4000) + 3000; // 2-3 seconds
  }
   
onReceive(LoRa.parsePacket());


//  xQueueReceive(dqueue, sbf.frame, 10000); //portMAX_DELAY)
//    Serial.println(sbf.pack.head[0]);
//    Serial.println(sbf.pack.head[1]);
//    Serial.println(sbf.pack.head[2]);
//    Serial.println(sbf.pack.head[3]);

//  LoRa.receive();
//  delay(3000);

}
