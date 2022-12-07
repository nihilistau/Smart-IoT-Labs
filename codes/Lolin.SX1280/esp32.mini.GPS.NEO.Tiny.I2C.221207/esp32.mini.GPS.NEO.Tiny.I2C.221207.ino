#include <Arduino.h>
#include <SoftwareSerial.h> 
#include <U8x8lib.h>  // bibliothèque à charger a partir de 


#include <TinyGPS++.h>

#define GPSRX 22  // 17
#define GPSTX 21  // 16
#define GPSBaud 9600 //GPS Baud rate

SoftwareSerial uart(GPSRX, GPSTX); 
TinyGPSPlus gps;

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15,4,16); // clock, data, reset

void disp(int d1, int d2, int d3, float lt, float lg) 
{ 
  char dbuf[16]; 
  u8x8.clear(); 
  Serial.println("titre"); 
  u8x8.drawString(0,1,"titre"); // 0 – colonne (max 15), 1 – ligne (max 7)
  sprintf(dbuf,"Hour: %d",d1); u8x8.drawString(0,2,dbuf); 
  sprintf(dbuf,"Minute: %d",d2); u8x8.drawString(0,3,dbuf); 
  sprintf(dbuf,"Second: %d",d3);u8x8.drawString(0,4,dbuf); 
  sprintf(dbuf,"Lat: %3.6f",lt); u8x8.drawString(0,5,dbuf); 
  sprintf(dbuf,"Lon: %3.6f",lg);u8x8.drawString(0,6,dbuf); 
  delay(6000); 
}


void setup(){  
  Serial.begin(9600); 
  delay(1000);
  uart.begin(GPSBaud);
  delay(1000);
  Serial.println("start UART");
  delay(1000);
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
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
            disp(gps.time.hour(), gps.time.minute(), gps.time.second(),gps.location.lat(),gps.location.lng() );
          }  
        }
      }
