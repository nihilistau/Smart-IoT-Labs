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
uint16_t timeout=10;
RTC_DATA_ATTR int counter=0;

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
    Serial.print("Sending packet: ");
  Serial.println(counter);
  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();
  counter++;
         Serial.println("Received DTACK"); //dtsnd_ack=0; 
         LoRa.sleep();  // stops LoRa modem and SPI bus connection
         //if(timeout==0) timeout=15;   // if received timeout is 0 set 15 secondes
         Serial.printf("Enters deep sleep for: %d sec\n",timeout); 
         esp_sleep_enable_timer_wakeup(1000*1000*timeout);  // in micro-seconds - timeout in seconds
         esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); 
         esp_deep_sleep_start(); 
}



void loop() {}
