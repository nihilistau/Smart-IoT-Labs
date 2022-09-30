
#include "WiFi.h"
#include <Wire.h>               
#include "SSD1306Wire.h"         
SSD1306Wire display(0x3c, 12, 14);  

void display_SSD1306(char *text) 
{
  char buff[256];
  strcpy(buff,text);
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "ETN - WiFi SCAN");
  display.drawString(0, 14, buff);
  display.display();
}

void setup()
{
  char buf[16];
  Serial.begin(9600);
  Wire.begin(12,14);
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(2000);
  Serial.println("Setup done");
}

void loop()
{
  int apmax=8, num;
  char buf[16];
  char tab[256];
  
  Serial.println("scan start");
  Serial.println("WiFi scan");
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("WiFi scan");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        //Serial.print(n);
        sprintf(buf,"WiFi AP#%d\n",n);
        Serial.println(buf);  
        delay(2000);
        if(n<apmax) num=n; else num=apmax;
        //Serial.println(" networks found");
        for (int i = 0; i < num; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            WiFi.SSID(i).toCharArray(buf,16);
            Serial.print(buf);
            if(!i) strcpy(tab,buf);else strcat(tab,buf);strcat(tab,"\n");
            display_SSD1306(tab);
            //Serial.println(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
    Serial.println("");
    delay(5000);
}
