#include <WiFi.h>
//#include "secrets.h"
#include "ThingSpeak.h"  
const char* ssid     = "PhoneAP";
const char* password = "smartcomputerlab";
WiFiClient  client;
unsigned long myChannelNumber = 1538804;
const char * myReadAPIKey = "20E9AQVFW7Z6XXOM";

void setup() {
    Serial.begin(9600);
    delay(10);
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(100); 
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

int statusCode=0;

void loop() { 
   // Connect or reconnect to WiFi
    while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");  delay(1000);     
    } 
    Serial.println("\nConnected.");
  // Read in field 2 of the private channel  
  int counter1 = ThingSpeak.readIntField(myChannelNumber, 2,myReadAPIKey);  
  // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Counter1: " + String(counter1));
  }
  else{
        Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  delay(20000); // No need to read the temperature too often.
}
