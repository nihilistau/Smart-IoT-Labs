
#include "Zanshin_BME680.h"  // Include the BME680 Sensor library
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

BME680_Class BME680;  ///< Create an instance of the BME680 class

void setup() 
{
  Serial.begin(9600);
  Wire.begin(29,28); //(29,28);     
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext,LOW);//set vext to high

  Serial.print("- Initializing BME680 sensor\n");
  while (!BME680.begin(I2C_STANDARD_MODE)) {  // Start BME680 using I2C, use first device found
    Serial.print("-  Unable to find BME680. Trying again in 5 seconds.\n");
    delay(5000);
  }  // of loop until device is located
  
  Serial.print("- Setting 16x oversampling for all sensors\n");
  BME680.setOversampling(TemperatureSensor, Oversample16);  // Use enumerated type values
  BME680.setOversampling(HumiditySensor, Oversample16);     // Use enumerated type values
  BME680.setOversampling(PressureSensor, Oversample16);     // Use enumerated type values
  Serial.print("- Setting IIR filter to a value of 4 samples\n");
  BME680.setIIRFilter(IIR4);  // Use enumerated type values
  Serial.print("- Setting gas measurement to 320\xC2\xB0\x43 for 150ms\n");  // "�C" symbols
  BME680.setGas(320, 150);  // 320�c for 150 milliseconds
}  // of method setup()

char buff1[32],buff2[32],buff3[32];

void loop() 
{
  static int32_t  temp, humidity, pressure, gas;  // BME readings
  BME680.getSensorData(temp, humidity, pressure, gas);  // Get readings
  Serial.println((int)temp/100);
  Serial.println((int)humidity/1000);
  Serial.println((int)pressure/100);
  sprintf(buff1,"T:%d",(int)temp/100);  
  sprintf(buff2,"H:%d",(int)humidity/1000);  
  sprintf(buff3,"P:%d",(int)pressure/100);
  displayOLED(buff1,buff2,buff3);
  delay(1200);  // Wait 10s             // of ignore first reading
}  // of method loop()
