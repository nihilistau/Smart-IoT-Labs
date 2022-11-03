#include "TinyMqtt.h"   // https://github.com/hsaturn/TinyMqtt
//#include <my_credentials.h>
#define PORT 1883
//#include <my_credentials.h>
const char *ssid     = "PhoneAP";
const char *pass = "smartcomputerlab";
#define LED_GREEN 4
#define LED_BLUE 5
MqttBroker broker(PORT);

/** Basic Mqtt Broker
  *
  *  +-----------------------------+
  *  | ESP                         |
  *  |       +--------+            | 
  *  |       | broker |            | 1883 <--- External client/s
  *  |       +--------+            |
  *  |                             |
  *  +-----------------------------+
  */
void setup() 
{
  pinMode(LED_GREEN, OUTPUT);pinMode(LED_BLUE, OUTPUT);
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
  digitalWrite(LED_GREEN, HIGH);
  Serial << "Connected to " << ssid << "IP address: " << WiFi.localIP() << endl;  
  broker.begin();
  Serial << "Broker ready : " << WiFi.localIP() << " on port " << PORT << endl;
  digitalWrite(LED_GREEN, LOW);                           
  digitalWrite(LED_BLUE, HIGH);    
}
void loop()
{
  broker.loop();
}
