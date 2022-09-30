#include <Wire.h>
#include "SparkFunHTU21D.h"
#include <SPI.h>              // include libraries
#include <LoRa.h>
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define freq    868E6   
#define sf 8
#define sb 125E3  

HTU21D mySensor;

union
{
 uint8_t frame[8]; // trames avec octets
  float  sensor[2];   // 2 valeurs en virgule flottante
} pack;

void setup() {
  Serial.begin(9600);
  delay(1000);
  mySensor.begin();  
    delay(1000);   
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa init OK");
LoRa.setSpreadingFactor(sf);
LoRa.setSignalBandwidth (sb);
  delay(1000);
}


void loop()  // la boucle de l’emetteur
{
Serial.println("New Packet");
  LoRa.beginPacket();                   // start packet
  pack.sensor[0]=mySensor.readHumidity();
  pack.sensor[1]=mySensor.readTemperature();
  Serial.println(  pack.sensor[0]);
  Serial.println(  pack.sensor[1]);
  LoRa.write(pack.frame,8);
  LoRa.endPacket();
delay(6000);
}

