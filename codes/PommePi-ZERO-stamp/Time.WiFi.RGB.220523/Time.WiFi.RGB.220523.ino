#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include "time.h"
long lastUpdate = millis();
long lastSecond = millis();

String hours, minutes, seconds;
int currentSecond, currentMinute, currentHour;


const char* ssid  = "PhoneAP";  //  your network SSID (name)
const char* password  = "smartcomputerlab";       // your network password

const float UTC_OFFSET = 0;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


Adafruit_NeoPixel led = Adafruit_NeoPixel(12, 9);

int ledh,ledm,leds;

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time"); return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.println(timeinfo.tm_hour);
  Serial.println(timeinfo.tm_min);
  Serial.println(timeinfo.tm_sec);
  ledh=(timeinfo.tm_hour+7)%12;
  ledm=(timeinfo.tm_min/5+7)%12;
  leds=(timeinfo.tm_sec/5+7)%12;
  led.clear();
  led.setPixelColor(leds, led.Color(0, 0, 255));
  led.setPixelColor(ledm, led.Color(0, 255, 0));
  led.setPixelColor(ledh, led.Color(255, 0, 0));
  led.show(); 
}

void setup()
{
  Serial.begin(9600);while(!Serial);Serial.println();
  led.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  led.clear(); // Set all pixel colors to 'off'
  // Set WiFi transmission power to 13dBm
  WiFi.setTxPower(WIFI_POWER_13dBm);  // optional
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop()
{
  delay(1000);
  printLocalTime();
}
