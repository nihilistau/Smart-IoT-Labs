#include <Wire.h>
#include "SHT21.h"

SHT21 SHT21;

float t,h;

void setup()
{  
//  pinMode(Vext,OUTPUT);
//  digitalWrite(Vext,LOW); //3V3 voltage output activated - Vext ON
//  Wire.begin(29,28);
//  SHT21.begin();
  Serial.begin(9600);
  pinMode(Vext,OUTPUT);
}

int dt, dh;

void loop()
{
  char buff[32];
 
  digitalWrite(Vext,LOW); // start power before activating Wire
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
  Serial.println(buff);
  Wire.end();   // end Wire before disconnecting power
  digitalWrite(Vext,HIGH);
  delay(3000);
}
