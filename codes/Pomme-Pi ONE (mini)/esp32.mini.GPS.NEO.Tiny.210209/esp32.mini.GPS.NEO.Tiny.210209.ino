#include <Arduino.h>
#include <SoftwareSerial.h> 

#include <TinyGPS++.h>

#define GPSRX 17
#define GPSTX 16
#define GPSBaud 9600 //GPS Baud rate

SoftwareSerial uart(GPSRX, GPSTX); 
TinyGPSPlus gps;
void setup(){  
  Serial.begin(9600); 
  delay(1000);
  uart.begin(GPSBaud);
  delay(1000);
  Serial.println("start UART");
  delay(1000);
}

  
  int i=0,ihour;
  char gpst[2048],ttime[16],ftime[16],hour[8],nhour[2];
  char *ptr=NULL;
  
  void loop(){  
    while (uart.available() > 0)
      {    
        gps.encode(uart.read());    
        if (gps.location.isUpdated())
          {      
            Serial.print("Latitude= "); 
            Serial.print(gps.location.lat(), 6);      
            Serial.print(" Longitude= "); 
            Serial.println(gps.location.lng(), 6);     
            Serial.print("Date:");
            Serial.print(gps.date.month());
            Serial.print("/");      
            Serial.print(gps.date.day());
            Serial.print("/");      
            Serial.print(gps.date.year()); 
            Serial.print("   Time:");    
            if (gps.time.hour() < 10) 
              Serial.print("0");      
            Serial.print(gps.time.hour()); 
            Serial.print(":");    
            if (gps.time.minute() < 10) 
              Serial.print("0");      
            Serial.print(gps.time.minute());
            Serial.print(":");      
            Serial.println(gps.time.second());    
          }  
        }
      }
