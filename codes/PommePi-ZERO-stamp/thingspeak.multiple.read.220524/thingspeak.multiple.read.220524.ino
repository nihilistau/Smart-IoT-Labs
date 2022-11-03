#include <WiFi.h>
#include "ThingSpeak.h"  
const char* ssid     = "PhoneAP";
const char* password = "smartcomputerlab";
WiFiClient  client;
unsigned long myChannelNumber = 1538804;
const char * myReadAPIKey = "20E9AQVFW7Z6XXOM";
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

int field[8] = {1,2,3,4,5,6,7,8};
// Initialize our values
int number1,number2,number3,number4;

void loop() {    
   // Connect or reconnect to WiFi
    while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");   delay(1000);     
    } 
    Serial.println("\nConnected.");
    int statusCode = ThingSpeak.readMultipleFields(myChannelNumber,myReadAPIKey);    
    if(statusCode == 200)
    {
     number1 = ThingSpeak.getFieldAsInt(field[0]); // Field 1
     number2 = ThingSpeak.getFieldAsInt(field[1]); // Field 1
     number3 = ThingSpeak.getFieldAsInt(field[2]); // Field 1
     number4 = ThingSpeak.getFieldAsInt(field[3]); // Field 1
     String createdAt = ThingSpeak.getCreatedAt(); // Created-at timestamp
     Serial.println(number1);Serial.println(number2);
     Serial.println(number3);Serial.println(number4);
     Serial.println(createdAt);
    }
    else{
      Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
    }
    Serial.println();
    delay(20000);    
}
