HardwareSerial uart(0);
#include <Wire.h>               // SDA-7, SCL-8
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"

SSD1306Wire display(0x3c, 7, 8); 

#include <Adafruit_NeoPixel.h>
#define PIN        2 
#define NUMPIXELS  1 

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
// RX0-21, TX0-20

void display_SSD1306(char* d1) 
{
  char buff[64];
  display.init();
  //display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Smart IoT DevKit");
  display.drawString(0, 28, d1);
  display.setFont(ArialMT_Plain_10);
  display.drawString(20, 52, "SmartComputerLab");
  display.display();
}

void setup() 
{
uart.begin(9600, SERIAL_8N1, 21, 20); //RX0, Tx0
pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
pixels.clear(); // Set all pixel colors to 'off'
Wire.begin(7,8);
delay(1000);
}

int i=0,ihour;
char gpst[2048],ttime[16],ftime[16],hour[8],nhour[2];
char *ptr=NULL;
uint8_t buff[32];



void loop()
{
i=0;
memset(gpst,0x00,2048);
while (uart.available() > 0)
  {
  char gpsData = uart.read();
  gpst[i]=(char)gpsData;i++;
  }
  if(i && i<100)
  {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); pixels.show(); 
  }
    if(i && i>100)
  {
    pixels.setPixelColor(0, pixels.Color(0, 0, 255)); pixels.show();
    ptr=strstr(gpst,"$GPGGA");
    memcpy(buff,ptr,13); memcpy(buff,"Time :",6);
    display_SSD1306((char*)buff);
  }
  delay(200); 
  pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  pixels.show(); 
  delay(500); 
}
