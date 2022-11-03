#include <WiFi.h>
//#include "secrets.h"
#include "ThingSpeak.h" 
const char* ssid     = "PhoneAP";
const char* password = "smartcomputerlab";
WiFiClient  client;
unsigned long myChannelNumber = 1538804;
const char * myWriteAPIKey = "YOX31M0EDKO0JATK";
int number = 0;

void setup() {
    Serial.begin(9600);
    delay(10);
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);  Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(100); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  // Connect or reconnect to WiFi
    while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(1000);     
    } 
    Serial.println("\nConnected.");
  int x = ThingSpeak.writeField(myChannelNumber, 1, number, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  // change the value
  number++;
  if(number > 99){
    number = 0;
  }  
  delay(20000); // Wait 20 seconds to update the channel again
}
