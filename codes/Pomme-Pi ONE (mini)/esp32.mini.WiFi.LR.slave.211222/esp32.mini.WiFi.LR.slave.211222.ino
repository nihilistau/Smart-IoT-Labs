// based on https://gist.github.com/yaqwsx/ac662c9b600ef39a802da0be1b25d32d
// 2018.07.14 jnogues@gmail.com, Jaume Nogués, rPrim Tech
// This sketch shows the use of 802.11 LR (Low Rate)
// slave.ino

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>

#include <Wire.h>                
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"

SSD1306Wire display(0x3c, 12, 14); 

const char* ssid = "ESP32_LR";//AP ssid
const char* password = "smartcomputerlab";//AP password

WiFiUDP udp;

const char *toStr( wl_status_t status ) {
    switch( status ) {
    case WL_NO_SHIELD: return "No shield";
    case WL_IDLE_STATUS: return "Idle status";
    case WL_NO_SSID_AVAIL: return "No SSID avail";
    case WL_SCAN_COMPLETED: return "Scan compleded";
    case WL_CONNECTED: return "Connected";
    case WL_CONNECT_FAILED: return "Failed";
    case WL_CONNECTION_LOST: return "Connection lost";
    case WL_DISCONNECTED: return "Disconnected";
    }
    return "Unknown";
}

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
    Serial.begin( 9600 );
    Wire.begin(12,14);
    Serial.println("Wire started");
    Serial.println( "Slave" );
    pinMode(22, OUTPUT);//bultin Led, for debug

    //We start STA mode with LR protocol
    //This ssid is not visible whith our regular devices
    WiFi.mode( WIFI_STA );//for STA mode
    //if mode LR config OK
    int a= esp_wifi_set_protocol( WIFI_IF_STA, WIFI_PROTOCOL_LR );
    if (a==0)
    {
      Serial.println(" ");
      Serial.print("Error = ");
      Serial.print(a);
      Serial.println(" , Mode LR OK!");
    }
    else//if some error in LR config
    {
      Serial.println(" ");
      Serial.print("Error = ");
      Serial.print(a);
      Serial.println(" , Error in Mode LR!");
    }
      
    WiFi.begin(ssid, password);//this ssid is not visible

    //Wifi connection, we connect to master
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }

    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  
    udp.begin(8888);
}


void loop() {
     //problems whith connection
    if ( WiFi.status() != WL_CONNECTED ) 
    {
        Serial.println( "|" );
        int tries = 0;
        WiFi.begin( ssid, password );
        while( WiFi.status() != WL_CONNECTED ) {
            tries++;
            if ( tries == 5 )
                return;
            Serial.println( toStr( WiFi.status() ) );
            delay( 1000 );
        }
        Serial.print( "Connected " );
        Serial.println( WiFi.localIP() );
    }
    //if connection OK, execute command 'b' from master
    int size = udp.parsePacket();
    if ( size == 0 )
        return;
    char c = udp.read();
    if ( c == 'b' ){
        digitalWrite(22, !digitalRead(22));//toggle Led
        Serial.println("RECEIVED!");
        Serial.println(millis());
        display_SSD1306("WiFi LR","RECEIVED IT!!");
    }
    udp.flush();
}
