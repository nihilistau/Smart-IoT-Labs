#include <Arduino.h>
#include <SoftwareSerial.h> 
#include <TinyGPS++.h>

#include <TFT_eSPI.h> 
#include <SPI.h>
#define TFT_GREY 0x5AEB // New colour

TFT_eSPI tft = TFT_eSPI();   

#define GPSRX 17
#define GPSTX 16
#define GPSBaud 9600 //GPS Baud rate

SoftwareSerial uart(GPSRX, GPSTX); 
TinyGPSPlus gps;

int count=0;
float lat,lng;
int year,mon,day,hour,minute,sec;


void TFT_Display()
{
char buff[32],dbuff[32],tbuff[32]; 
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 10, 2);
    tft.setTextColor(TFT_BLUE,TFT_BLACK);  tft.setTextSize(2);
    tft.println("TOPGNSS - 2M");
    tft.setCursor(22, 50, 2);
    tft.setTextColor(TFT_RED,TFT_BLACK);  tft.setTextSize(2);
    sprintf(buff,"LAT: %3.6f\n  LNG: %3.6f\n",lat,lng);
    tft.println(buff);
    tft.setCursor(10,120,2);
    tft.setTextColor(TFT_GREEN,TFT_BLACK);  tft.setTextSize(2);
    sprintf(dbuff,"Date: %2.2d/%2.2d/%2.2d\n",mon,day,year);
    tft.println(dbuff);
    tft.setCursor(20,150,2);
    tft.setTextColor(TFT_YELLOW,TFT_BLACK);  tft.setTextSize(2);
    sprintf(tbuff,"GMTime: %2.2d:%2.2d:%2.2d\n",hour,minute,sec);
    tft.println(tbuff);
    tft.setCursor(120,210,1);
    tft.setTextColor(TFT_GREEN,TFT_BLACK);  tft.setTextSize(1);
    tft.println("SmartComputerLab");
}

void setup()
{  
  Serial.begin(9600); 
  delay(1000);
  tft.init();
  tft.setRotation(1);
  uart.begin(GPSBaud);
  delay(1000);
  Serial.println("start UART");
  delay(1000);
}

  
  char *ptr=NULL;
  
  void loop(){  

    while (uart.available() > 0)
      {    
        gps.encode(uart.read());    
        if (gps.location.isUpdated())
          {      
            Serial.print("Latitude= "); 
            lat=gps.location.lat();
            Serial.print(lat, 6);      
            Serial.print(" Longitude= "); 
            lng=gps.location.lng();
            Serial.println(lng, 6);     
            Serial.print("Date:");
            mon=gps.date.month();
            Serial.print(mon);
            Serial.print("/"); 
            day=gps.date.day();     
            Serial.print(day);
            Serial.print("/");
            year=gps.date.year();     
            Serial.print(year); 
            Serial.print("   Time:");    
            if (gps.time.hour() < 10) 
              Serial.print("0");  
            hour= gps.time.hour();     
            Serial.print(hour); 
            Serial.print(":");    
            if (gps.time.minute() < 10) 
              Serial.print("0"); 
            minute=gps.time.minute();       
            Serial.print(minute);
            Serial.print(":");
            sec= gps.time.second();     
            Serial.println(sec); 
            TFT_Display();
               
          }  
        }
      }

      //  USER DEFINED SETTINGS in User_Setup.h  for ST7789 display

#define ST7789_2_DRIVER    // Minimal configuration option, define additional parameters below for this display
#define TFT_WIDTH  240 // ST7789 240 x 240 and 240 x 320
#define TFT_HEIGHT 240 // ST7789 240 x 240

// Section 2. Define the pins that are used to interface with the display here

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
//#define TFT_CS   -1  //5  // Chip select control pin
#define TFT_DC   26 //5 // 15 //2 //2 //2  // Data Command control pin
#define TFT_RST  15 // 5 //4 //4  // Reset pin (could connect to RST pin)
//#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST


// Section 3. Define the fonts that are to be used here

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts
#define SMOOTH_FONT

// Section 4. Other options
// #define SPI_FREQUENCY  20000000
#define SPI_FREQUENCY  27000000

      
