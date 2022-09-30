
#include <Wire.h>                
#include "SSD1306Wire.h"         
SSD1306Wire display(0x3c, 12, 14);    
#include "SHT21.h"
SHT21 SHT21;

float stab[4]= {0.0,0.0,0.0,0.0};

void get_SHT21() 
{
    SHT21.begin();
    delay(1000);
    stab[0]=SHT21.getTemperature();
    delay(100);
    stab[1]=SHT21.getHumidity();
    Serial.printf("T:%2.2f, H:%2.2f\n",stab[0],stab[1]);
}

void display_SSD1306(float d1,float d2,float d3,float d4) 
{
  char buff[64];
  display.init();
  //display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "ETN - IoT DevKit");
  display.setFont(ArialMT_Plain_10);
  sprintf(buff,"T: %2.2f, H: %2.2f\nd3: %2.2f, d4: %2.2f",d1,d2,d3,d4);
  display.drawString(0, 22, buff);
  display.drawString(20, 52, "SmartComputerLab");
  display.display();
}

void setup() {
  Serial.begin(9600);
  Wire.begin(12,14);
  Serial.println();

}

void loop()
{
  Serial.println("in the loop");
  get_SHT21(); delay(2000);
  display_SSD1306(stab[0],stab[1],stab[2],stab[3]);
  delay(2000);
}
