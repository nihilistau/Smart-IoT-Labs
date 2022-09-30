//#define THINGER_SERVER "192.168.1.49"
#define THINGER_SERVER "90.49.255.63"
//#define THINGER_PORT 80
#define THINGER_PORT 25202
#define _DEBUG_ 
//#define _DISABLE_TLS_

//#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ThingerESP32.h>

#include "DHT.h"

// Define and initialise the sensor
#define DHTPIN 17
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define USERNAME "smtr"
#define DEVICE_ID "esp32dht22"
#define DEVICE_CREDENTIAL "If4KWw1NLksr"

#define SSID "Livebox-08B0"
#define SSID_PASSWORD "G79ji6dtEptVTPWmZP"

// define your board pin here
#define LED_PIN 25

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

float humi,temp;

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  thing.add_wifi(SSID, SSID_PASSWORD);
  //Serial.println("set wifi");

  // Define the 'thing' with a name and data direction
  thing["dht11"] >> [](pson& out){
    // Add the values and the corresponding code
    humi = dht.readHumidity();
    temp = dht.readTemperature();
    out["humidity"] = humi;
    out["celsius"] = temp;
    Serial.println(humi);    Serial.println(temp);
  };
}
void loop() {
  thing.handle();
  //Serial.println("in the loop");
}
