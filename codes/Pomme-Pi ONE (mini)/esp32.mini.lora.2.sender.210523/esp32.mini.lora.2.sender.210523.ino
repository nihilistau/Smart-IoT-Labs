//Attention low upload speed- 460800

#include <LoRa.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"

// Initialize the OLED display using Arduino Wire:
SSD1306Wire display(0x3c, 12, 14);   // ADDRESS, SDA, SCL

String receivedText;
String receivedRssi;
// with LoRa modem RFM95 and green RFM board - int and RST to solder

#define SS      4      // 5 // 26     // D0 - to NSS
#define RST     13     //15  //16     // D4  - RST
#define DI0     27    //26      // D8 - INTR

#define SCK     18      // D5 - CLK
#define MISO    19      // D6 - MISO
#define MOSI    23      // D7 - MOSI
#define BAND    434E6

int sf=7;
long sbw=125E3;

char dbuff[24];

void setup() {
  Serial.begin(9600);
  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);

  Serial.begin(9600);
  delay(1000);
  Serial.println();Serial.println();
  Serial.println("Starting LoRa Sender");
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

   display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Hello LoRa");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 16, "LoRa sender");
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 32, "Hello LoRa");
    display.display();

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Starting LoRa failed");
    display.display();
    while (1);
  }
  else 
   {
   Serial.println("Starting LoRa ok!");
   display.clear();
   display.setFont(ArialMT_Plain_10);
   display.drawString(0, 0, "Starting LoRa OK");
   display.display();
   LoRa.setSpreadingFactor(sf);
   LoRa.setSignalBandwidth(sbw);
   }
}

int counter=0;

void loop() {
char buff[32];
  Serial.print("Sending packet: ");
  Serial.println(counter);
  display.clear();
  display.drawString(0, 2, "Sending packet");
  sprintf(buff,"Count:  %d",counter);
  display.drawString(0, 24, buff);
  display.display();
  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter); 
  LoRa.endPacket();

  counter++;

  delay(5000);

}
