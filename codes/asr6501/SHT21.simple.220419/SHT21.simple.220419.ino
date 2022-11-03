#include <Wire.h>
#include "SHT21.h"
SHT21 SHT21;
int t,h;
void setup()
{
  Serial.begin(9600);
  Wire.begin(); //(29,28);     
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext,LOW);//set vext to high
}

int dt,dh;
void loop()
{
char buff[32];
t=(int)SHT21.getTemperature();
h=(int)SHT21.getHumidity();
Serial.print("Humidity(%RH): ");
Serial.print(h);
Serial.print("    Temperature(C): ");
Serial.println(t);
dt=(int)((t-(int)t)*100.0); dh=(int)((h-(int)h)*100.0);
sprintf(buff,"T:%d.%d, H:%d.%d\n",(int)t,dt,(int)h,dh);
Serial.println(buff);
delay(1000);
}
