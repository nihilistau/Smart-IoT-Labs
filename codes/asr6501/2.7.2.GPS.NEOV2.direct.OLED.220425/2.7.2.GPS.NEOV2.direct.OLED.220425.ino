#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "HT_SSD1306Wire.h"
#include "Wire.h"
#include <softSerial.h>

// The serial connection to the GPS device
softSerial softwareSerial(GPIO3 /*TX pin*/, GPIO5 /*RX pin*/);  // GPIO5 (33) , GPIO3 (8)

SSD1306Wire  display(0x3c, 500000, SDA, SCL, GEOMETRY_128_64, -1);  // addr , freq , i2c group , resolution , rst

void displayOLED(char *line1, char *line2, char *line3)
{ 
    display.init();
    display.flipScreenVertically();display.clear();
    display.drawString(20, 50, "SmartComputerLab" );
    display.drawString(0, 0,line1 );
    display.drawString(0,16,line2 );
    display.drawString(0,32,line3);
    display.display();
}

void setup()
{

pinMode(Vext, OUTPUT);
digitalWrite(Vext, LOW);
delay(500);
Serial.begin(9600);
Wire.begin(29,28);
softwareSerial.begin(9600);
delay(1000);
Serial.println("Normal serial init");
}
char *ptr, gmt[12],clarg[12], clong[12],dgmt[24],dclarg[24], dclong[24];
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
//Serial.print("Received data from software Serial:");
Serial.println(serialbuffer);
ptr=strstr(serialbuffer,"RMC,");
strncpy(gmt,ptr+4,6); Serial.print("GMT:");Serial.println(gmt);
ptr=strstr(serialbuffer,",A,");
strncpy(clarg,ptr+3,10); Serial.print("Larg:");Serial.println(clarg);
ptr=strstr(serialbuffer,",N,");
strncpy(clong,ptr+3,10); Serial.print("Long:");Serial.println(clong);
Serial.println();
sprintf(dgmt,"GMT:%s",gmt);sprintf(dclarg,"LAT:%s",clarg);sprintf(dclong,"LNG:%s (W)",clong);
displayOLED(dgmt, dclarg, dclong);
}
}
delay(600);
}
