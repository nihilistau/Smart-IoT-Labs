#include <Adafruit_NeoPixel.h>
#define PIN        2 
#define NUMPIXELS  1 
#include "TinyMqtt.h"   // https://github.com/hsaturn/TinyMqtt
//#include <my_credentials.h>
#define PORT 1883
//#include <my_credentials.h>
//const char *ssid     = "PhoneAP";
//const char *pass = "smartcomputerlab";

const char* ssid = "Livebox-08B0";
const char* pass = "G79ji6dtEptVTPWmZP";

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

MqttBroker broker(PORT);

void setup() 
{
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixel colors to 'off'
  Serial.begin(9600);while(!Serial);Serial.println();
  // Set WiFi transmission power to 13dBm
  WiFi.setTxPower(WIFI_POWER_13dBm);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
 
  while (WiFi.status() != WL_CONNECTED) {   
    Serial << '.';
    delay(500);
  }
  pixels.setPixelColor(0, pixels.Color(0, 150, 0));pixels.show(); 
  Serial << "Connected to " << ssid << "IP address: " << WiFi.localIP() << endl;  
  broker.begin();
  Serial << "Broker ready : " << WiFi.localIP() << " on port " << PORT << endl;
  pixels.setPixelColor(0, pixels.Color(0, 150, 0));pixels.show();                           
   pixels.setPixelColor(0, pixels.Color(0, 0, 150));pixels.show();    
}
void loop()
{
  broker.loop();
}
