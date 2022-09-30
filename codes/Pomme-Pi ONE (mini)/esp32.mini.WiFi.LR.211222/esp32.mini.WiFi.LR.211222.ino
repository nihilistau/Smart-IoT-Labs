#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>

#include <Wire.h>                
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"

SSD1306Wire display(0x3c, 12, 14); 


char *ssid = "Livebox-08B0";
char *pass = "G79ji6dtEptVTPWmZP";
const char* ssidRouter = "ESP32_LR";//STA router ssid
const char* passwordRouter = "smartcomputerlab";//STA router password
WiFiUDP udp;

void display_SSD1306(char *s1, char *s2) 
{
  char buff[64];
  display.init();
  //display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, s1);
  display.setFont(ArialMT_Plain_10);
  sprintf(buff,"%s",s2);
  display.drawString(0, 22, buff);
  display.drawString(20, 52, "SmartComputerLab");
  display.display();
}


void setup() {
    pinMode(22, OUTPUT);//builtin Led, for debug
    digitalWrite(22, HIGH);
    Serial.begin(9600);
    Wire.begin(12,14);
    Serial.println("Wire started");
    Serial.println( "Master" );

    //first, we start STA mode and connect to router
//    WiFi.mode( WIFI_AP_STA );
//    WiFi.begin(ssid, pass);
//    
//    
//    //Wifi connection
//    while (WiFi.status() != WL_CONNECTED) 
//    {
//      delay(500);
//      Serial.print(".");
//    }
//
//    Serial.println("Router WiFi connected");
//    Serial.print("IP address: ");
//    Serial.println(WiFi.localIP());
  
    //second, we start AP mode with LR protocol
    //This AP ssid is not visible whith our regular devices
    WiFi.mode( WIFI_AP );//for AP mode
    //here config LR mode
    int a= esp_wifi_set_protocol( WIFI_IF_AP, WIFI_PROTOCOL_LR );
    Serial.println(a);
    WiFi.softAP(ssidRouter,passwordRouter); 
    Serial.println( WiFi.softAPIP() );
    Serial.println("#");//for debug
    delay( 1000 );
    digitalWrite(22, LOW); 
    udp.begin(8888);
}

void loop() 
{
    udp.beginPacket( { 192, 168, 4, 255 }, 8888 );//send a broadcast message
    udp.write( 'b' );//the payload
    digitalWrite(22, !digitalRead(22));
    
    if ( !udp.endPacket() ){
        Serial.println("NOT SEND!");
        delay(100);
        ESP.restart(); // When the connection is bad, the TCP stack refuses to work
    }
    else{
          Serial.println("SEND IT!!"); 
          display_SSD1306("WiFi LR","SEND IT!!");
    }     
    
    delay( 1000 );//wait a second for the next message
}
