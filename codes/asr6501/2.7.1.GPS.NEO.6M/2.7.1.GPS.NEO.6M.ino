#include <Arduino.h>
#include <softSerial.h>
softSerial softwareSerial(GPIO5 /*TX pin*/, GPIO3 /*RX pin*/);  // GPIO5 (33) , GPIO3 (8)

void setup()
{
pinMode(Vext, OUTPUT);
digitalWrite(Vext, LOW);
delay(500);
Serial.begin(9600);
softwareSerial.begin(9600);
delay(1000);
Serial.println("Normal serial init");
}
char *ptr, gmt[12],clarg[12], clong[12];

void loop()
{
  if(softwareSerial.available())
  {
  char serialbuffer[256] = {0};
  int i = 0;
  while (softwareSerial.available() && i<256)
    {
    serialbuffer[i] = (char)softwareSerial.read();
    i++;
    }
  serialbuffer[i] = '\0';
  if(serialbuffer[0])
    {
    Serial.println(serialbuffer);
    ptr=strstr(serialbuffer,"RMC,");
    strncpy(gmt,ptr+4,6); Serial.print("GMT:");Serial.println(gmt);
    ptr=strstr(serialbuffer,",A,");
    strncpy(clarg,ptr+3,10); Serial.print("Larg:");Serial.println(clarg);
    ptr=strstr(serialbuffer,",N,");
    strncpy(clong,ptr+3,10); Serial.print("Long:");Serial.println(clong);
    Serial.println();
    }
  }
delay(200);
}
