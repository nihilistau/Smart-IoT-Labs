
 
// Include the correct display library

// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>               // SDA-7, SCL-8
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"

SSD1306Wire display(0x3c, 7, 8);   // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h

void display_SSD1306(float d1,float d2,float d3,float d4) 
{
  char buff[64];
  display.init();
  //display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "ETN - IoT DevKit");
  display.setFont(ArialMT_Plain_10);
  sprintf(buff,"d1: %2.2f, d2: %2.2f\nd3: %2.2f, d4: %2.2f",d1,d2,d3,d4);
  display.drawString(0, 22, buff);
  display.drawString(20, 52, "SmartComputerLab");
  display.display();
}

void setup() {
  Serial.begin(9600);
  Wire.begin(7,8);
  Serial.println("Wire started");
}


float v1=0.0,v2=0.0,v3=0.0,v4=0.0;

void loop()
{
  Serial.println("in the loop");
  display_SSD1306(v1,v2,v3,v4);
  v1=v1+0.1;v2=v2+0.2;v3=v3+0.3;v4=v4+0.4;
  delay(2000);
}
