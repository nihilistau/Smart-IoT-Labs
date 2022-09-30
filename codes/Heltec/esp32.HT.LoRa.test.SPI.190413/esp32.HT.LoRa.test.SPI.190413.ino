

#include <SPI.h>
#include <LoRa.h>



//#define SCK     5    // GPIO5  -- SX127x's SCK
//#define MISO    19   // GPIO19 -- SX127x's MISO
//#define MOSI    27   // GPIO27 -- SX127x's MOSI
//#define SS      18   // GPIO18 -- SX127x's CS
//#define RST     14   // GPIO14 -- SX127x's RESET
//#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)


#define SCK     17    // GPIO5  -- SX127x's SCK  17
#define MISO    13   // GPIO19 -- SX127x's MISO 13
#define MOSI    12   // GPIO27 -- SX127x's MOSI  12
#define SS      23   // GPIO18 -- SX127x's CS
#define RST     25   // GPIO14 -- SX127x's RESET  25
#define DI0     2  // GPIO26 -- SX127x's IRQ(Interrupt Request)  2

//#define SD_CS 23
//#define SD_SCK 17
//#define SD_MOSI 12
//#define SD_MISO 13


//SPIClass lora_spi(HSPI);

SPIClass * hspi = NULL;


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
//long freq=868500E3; 
long freq=434500E3; 



int api=0, constep=0, apcon=0;
 
void setup() {
  Serial.begin(9600);
  hspi = new SPIClass(HSPI);
  Serial.println(sf);
  Serial.println(sb);
  Serial.println(freq);
  
  pinMode(23, OUTPUT);  // CS output
  pinMode(17, OUTPUT);  // CLK output
  pinMode(2, INPUT);  // recv interrupt
  

 
 SPI.begin(SCK,MISO,MOSI,SS);
 LoRa.setPins(SS,RST,DI0);

  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sb);
  LoRa.setCodingRate4(8);
  LoRa.enableCrc();

  Serial.println("LoRa receiver started"
  );

}



int i=0;

void loop() {

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
      Serial.println(rssi);
      delay(4000);
    }  
     // waiting for packet in loop
}
