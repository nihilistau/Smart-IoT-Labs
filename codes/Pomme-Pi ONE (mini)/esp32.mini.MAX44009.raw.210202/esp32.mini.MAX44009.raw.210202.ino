#include <Wire.h>
#define Addr 0x4A
 
void setup()
{
 
Wire.begin(12,14);
// Initialise serial communication
Serial.begin(9600);
 
Wire.beginTransmission(Addr);
Wire.write(0x02);
Wire.write(0x40);
Wire.endTransmission();
delay(300);
}

void loop()
{
unsigned int data[2];
Wire.beginTransmission(Addr);
Wire.write(0x03);
Wire.endTransmission();
 
// Request 2 bytes of data
Wire.requestFrom(Addr, 2);
 
// Read 2 bytes of data luminance msb, luminance lsb
if (Wire.available() == 2)
{
data[0] = Wire.read();
data[1] = Wire.read();
}
 
// Convert the data to lux
int exponent = (data[0] & 0xF0)>>4;
int mantissa = ((data[0] & 0x0F) << 4) | (data[1]& 0x0F);
float luminance = pow(2, exponent) * mantissa * 0.045;
 
Serial.print("Ambient Light luminance :");
Serial.print(luminance);
Serial.println(" lux");
delay(500);
}
