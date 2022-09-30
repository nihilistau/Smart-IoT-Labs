#include "ThingSpeak.h"
#include <WiFi.h>
#include <SoftwareSerial.h>
#include <Wire.h>   
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
SSD1306  display(0x3c, 12, 14); // SDA - 21, SCL - 22 

#include <SoftwareSerial.h>
SoftwareSerial uart;

char *ssid =     "Livebox-08B0";
char *pass = "G79ji6dtEptVTPWmZP";
//char * ssid2 = "PhoneAP";          //  your network SSID (name) 
//char * password2 = "smartcomputerlab";   // your network passw

WiFiClient  client;

//unsigned long myChannelNumber = 1243378;   
//const char *myWriteAPIKey="46FEZYSZLXW1P0SK" ;

unsigned long myChannelNumber = 9;   
const char *myWriteAPIKey="8ANOOJDK7T118TP7" ;

union 
{
  uint8_t frame[28];
  struct
    {
      uint8_t head[4];
      int nmac; int dmac;
      int minrssi;
      int maxrssi;
      long longitude;
      long latitude;
    } pack;
} rdf;


char dbuff[128];

void setup() {
char dbuff[128];  
Serial.begin(9600);
display.init();
display.flipScreenVertically();
display.setFont(ArialMT_Plain_10);  // _16, _24
display.setTextAlignment(TEXT_ALIGN_LEFT);
//display.drawString(0, 10, "Left aligned (0,10)");
display.drawString(0, 4, "UART-WiFi relay");
display.display();
WiFi.disconnect(true);
WiFi.mode(WIFI_STA);
WiFi.begin(ssid, pass);
while (WiFi.status() != WL_CONNECTED) {
  delay(500);Serial.print(".");
  }
IPAddress ip = WiFi.localIP();
Serial.print("IP Address: ");
Serial.println(ip);
sprintf(dbuff,"IP:%3.3d.%3.3d.%3.3d.%3.3d",ip[0],ip[1],ip[2],ip[3]);
display.drawString(0, 16, dbuff);display.display();
ThingSpeak.begin(client);
uart.begin(9600, SWSERIAL_8N1, 16, 17); // RxD, TxD

}

int i=0,j=0,n=0,x=0;

void loop()
{
char dbuff[128];  
i=0;
while(uart.available() > 0) 
  {
  rdf.frame[i]=uart.read();i++; n=i;
  }
if(i==28)
  {    
  boolean ret;
  if(WiFi.status() != WL_CONNECTED) {  delay(500);  }
  Serial.println("WiFi connected"); 
  ret=ThingSpeak.begin(client); // connexion (TCP) du client au serveur
  if(ret)
    {
    delay(1000);
    Serial.println("ThingSpeak begin"); Serial.println(i);//Serial.println((char*)rdf.frame);
    Serial.println(rdf.pack.nmac);Serial.println(rdf.pack.minrssi);Serial.println(rdf.pack.maxrssi);
    Serial.println("Fields update");
    if(rdf.pack.nmac>0 && rdf.pack.nmac<1000 && rdf.pack.minrssi> -150 && rdf.pack.minrssi< 0 && rdf.pack.maxrssi> -150 && rdf.pack.maxrssi< 0)
      {
      ThingSpeak.setField(1, (int)rdf.pack.nmac);
      ThingSpeak.setField(2, (int)rdf.pack.dmac-1);
      ThingSpeak.setField(3, (int)rdf.pack.minrssi);
      ThingSpeak.setField(4, (int)rdf.pack.maxrssi);
      x=ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
      if(x==200)
      {
      display.clear();display.display();
      display.setFont(ArialMT_Plain_16);memset(dbuff,0x00,128);
      sprintf(dbuff,"SMAC:%3.3d",(int)rdf.pack.nmac);display.drawString(0, 0, dbuff);
      display.setFont(ArialMT_Plain_16);memset(dbuff,0x00,128);
      sprintf(dbuff,"DMAC:%3.3d",(int)rdf.pack.dmac);display.drawString(0, 20, dbuff);
      display.setFont(ArialMT_Plain_10);memset(dbuff,0x00,128);
      sprintf(dbuff,"min/max:%3.3d/%3.3d",(int)rdf.pack.minrssi,(int)rdf.pack.maxrssi);display.drawString(0, 40, dbuff);
      display.display();
      }
      else 
      {

        Serial.println("Update error"); 
      }
        
    delay(5000);uart.flush();  // must be shorter than the sender cycle !
    }
  } 
  }   
delay(200);
}
