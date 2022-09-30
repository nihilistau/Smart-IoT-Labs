#include <SPI.h>              // include libraries
#include <LoRa.h>

#define SS       5 // 26     // D0 - to NSS
#define RST     15  //16     // D4  - RST
#define DI0     26      // D8 - INTR
#define SCK     18      // D5 - CLK
#define MISO    19      // D6 - MISO
#define MOSI    23      // D7 - MOSI
#define BAND    868E6

int sf=7;
long sbw=125E3;

void setup() {
  Serial.begin(9600);  Serial.println(); 
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  delay(100);Serial.println(); 
  Serial.println("LoRa init succeeded.");
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sbw);
  Serial.printf("SF set to: %d, Bandwidth set to: %d Hz\n",sf,sbw);
  delay(100);
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
