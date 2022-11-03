
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "HT_SSD1306Wire.h"
#include "Wire.h"
// const uint32_t SERIAL_SPEED{115200};  ///< Set the baud rate for Serial I/O
 
SSD1306Wire  display(0x3c, 500000, SDA, SCL, GEOMETRY_128_64, -1);
                    
void displayOLED(char *line1, char *line2, char *line3)
{
    
    display.init();
    display.flipScreenVertically();display.clear();
    display.drawString(20, 50, "SmartComputerLab" );
    display.drawString(0, 0,  line1 );
    display.drawString(0, 15, line2);
    display.drawString(0, 30, line3);
    display.display();
}


void setup() 
{
  Serial.begin(9600);
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext,LOW);//set vext to high
  Wire.begin(29,28); //(29,28);     

  delay(2000);
}  // of method setup()

char buff1[32],buff2[32],buff3[32];

void loop() 
{
  static int32_t  temp, humidity, pressure;  // BME readings
  temp=23; humidity=67; pressure= 1200;  // Get readings
  Serial.println((int)temp);
  Serial.println((int)humidity);
  Serial.println((int)pressure);  
  sprintf(buff1,"H:%d",(int)temp);
  sprintf(buff2,"H:%d",(int)humidity);  
  sprintf(buff3,"P:%d",(int)pressure);
  displayOLED(buff1,buff2,buff3);
  delay(1200);  // Wait 10s             // of ignore first reading
}  // of method loop()
