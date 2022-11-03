#include <Wire.h>
#include "SHT21.h"
#include "HT_SSD1306Wire.h"

SHT21 SHT21;

SSD1306Wire  display(0x3c, 100000, SDA, SCL, GEOMETRY_128_64, -1);
                    
void displayOLED(char *line1, char *line2, char *line3)
{ 
    Serial.println("in oled");
    display.init();
    display.flipScreenVertically();display.clear();
    display.drawString(20, 50, "SmartComputerLab" );
    display.drawString(0, 0,  line1 );
    display.drawString(0, 15, line2);
    display.drawString(0, 30, line3);
    display.display();
    delay(1000);  // display stays 1000 ms, before power cut-off
}

float t,h;

void setup()
{  
  Serial.begin(9600);
  pinMode(Vext,OUTPUT);
}

int dt, dh;

void loop()
{
  char buff[32],bufft[32],buffh[32];
  digitalWrite(Vext,LOW); //3V3 voltage output activated - Vext ON
  Wire.begin(29,28);
  SHT21.begin();
  delay(200);
  t=SHT21.getTemperature();
  h=SHT21.getHumidity();
  Serial.print("Humidity(%RH): ");
  Serial.print(h);
  Serial.print("     Temperature(C): ");
  Serial.println(t);
  dt=(int)((t-(int)t)*100.0);  dh=(int)((h-(int)h)*100.0);
  sprintf(buff,"T:%d.%d, H:%d.%d\n",(int)t,dt,(int)h,dh);
  sprintf(bufft,"T:%d.%d",(int)t,dt);
  sprintf(buffh,"H:%d.%d",(int)h,dh);
  Serial.println(buff);Serial.println(bufft);Serial.println(buffh);
  displayOLED(bufft,buffh," ");
  Wire.end();
  digitalWrite(Vext,HIGH);
  delay(5000);
}
