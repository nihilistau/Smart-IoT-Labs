//#define THINGER_SERVER "192.168.1.49"
#define THINGER_SERVER "90.49.255.63"
//#define THINGER_PORT 80
#define THINGER_PORT 25202
#define _DEBUG_ 
//#define _DISABLE_TLS_

//#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ThingerESP32.h>

#define USERNAME "smtr"
#define DEVICE_ID "esp32heltec"
#define DEVICE_CREDENTIAL "qmZKjy60Tm4e"

#define SSID "Livebox-08B0"
#define SSID_PASSWORD "G79ji6dtEptVTPWmZP"

// define your board pin here
#define LED_PIN 25

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  thing.add_wifi(SSID, SSID_PASSWORD);
  //Serial.println("set wifi");

  // digital pin control example (i.e. turning on/off a light, a relay, configuring a parameter, etc)
  thing["led"] << digitalPin(LED_PIN);

  // resource output example (i.e. reading a sensor value)
  thing["millis"] >> outputValue(millis());

  //Serial.println("end setup");

  // more details at http://docs.thinger.io/arduino/
}

void loop() {
  thing.handle();
  //Serial.println("in the loop");
}
