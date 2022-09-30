#include <WiFiClientSecure.h>
#include <ThingerESP32.h>

#define THINGER_SERVER "90.105.150.96"
#define THINGER_PORT 25202
#define _DEBUG_ 

#define USERNAME "bako"
#define DEVICE_ID "esp32led"
#define DEVICE_CREDENTIAL "chLvq4Mf!XZI"

#define SSID "Livebox-08B0"
#define SSID_PASSWORD "G79ji6dtEptVTPWmZP"

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup() {
  Serial.begin(9600);
  pinMode(25, OUTPUT);

  thing.add_wifi(SSID, SSID_PASSWORD);

  // digital pin control example (i.e. turning on/off a light, a relay, configuring a parameter, etc)
  thing["led"] << digitalPin(LED_BUILTIN);

  // resource output example (i.e. reading a sensor value)
  thing["millis"] >> outputValue(millis());

  // more details at http://docs.thinger.io/arduino/
}

void loop() {
  
  thing.handle();
}
