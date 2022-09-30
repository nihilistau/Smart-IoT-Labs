#include <ESP_WiFiManager.h> 
#include "ThingSpeak.h"

unsigned long myChannelNumber = 1;   
const char * myWriteAPIKey="HEU64K3PGNWG36C4" ;
WiFiClient  client;

void setup() {
  Serial.begin(9600);
  Serial.print("\nStarting AutoConnect_ESP32_minimal on "); 
  ESP_WiFiManager ESP_wifiManager("AutoConnectAP");
  ESP_wifiManager.autoConnect("AutoConnectAP");
  if (WiFi.status() == WL_CONNECTED) { Serial.print(F("Connected. Local IP: "));Serial.println(WiFi.localIP()); }
  else { Serial.println(ESP_wifiManager.getStatus(WiFi.status())); }

  ThingSpeak.begin(client); // connexion (TCP) du client au serveur
  delay(1000);
  Serial.println("ThingSpeak begin");
}

float temperature,humidity,tem,hum;

void loop() {
  delay(5000);
//  if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
//    WiFi.begin(ssid);
//    delay(500);        
//}
  Serial.println("Connecting to ThingSpeak.fr or ThingSpeak.com");
  // modify the ThingSpeak.h file to provide correct URL/IP/port
  WiFiClient client;
  delay(1000);
  ThingSpeak.begin(client);
  delay(1000);
  Serial.println("Fields update");
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  delay(6000);
  temperature+=0.1;
  humidity+=0.2;
  tem =ThingSpeak.readFloatField(myChannelNumber,1);
  Serial.print("Last temperature:"); 
  Serial.println(tem);
  delay(6000);
  hum =ThingSpeak.readFloatField(myChannelNumber,2);
  Serial.print("Last humidity:"); 
  Serial.println(hum);
  delay(6000);
  tem =ThingSpeak.readFloatField(myChannelNumber,1);
  Serial.print("Last temperature:"); 
  Serial.println(tem);
  delay(6000);
  hum =ThingSpeak.readFloatField(myChannelNumber,2);
  Serial.print("Last humidity:"); 
  Serial.println(hum);
  delay(6000);
}
