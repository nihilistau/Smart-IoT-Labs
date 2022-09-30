/*
  UDPSendReceive.pde:
 This sketch receives UDP message strings, prints them to the serial port
 and sends an "acknowledge" string back to the sender

 A Processing sketch is included at the end of file that can be used to send
 and received messages for testing with a computer.

 created 21 Aug 2010
 by Michael Margolis

 This code is in the public domain.
 */


#include <SPI.h>          
#include <Ethernet2.h>
#include <EthernetUdp2.h>          
#include <Wire.h>                
#include "SSD1306Wire.h"         
SSD1306Wire display(0x3c, 12, 14); 

#define     ETH_RST        15 // 17  // 4
#define     ETH_CS          5
#define     ETH_SCLK       18
#define     ETH_MISO       19  //23
#define     ETH_MOSI       23  //19

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,172),rip(192,168,1,178);

unsigned int localPort = 8888;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;


void ethernetReset(const uint8_t resetPin)
{
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, HIGH);
    delay(250);
    digitalWrite(resetPin, LOW);
    delay(50);
    digitalWrite(resetPin, HIGH);
    delay(350);
}

void disp_sens(char *mess)
{
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0,0,"Terminal ETH");  // first 16 lines are yellow
  display.drawString(0,16,"192.168.1.172:8888");
  display.drawString(0,32,mess);

  display.display();
}

void setup() 
{
  Serial.begin(9600); delay(100);
  Wire.begin(12,14);delay(100);
  SPI.begin(ETH_SCLK, ETH_MISO, ETH_MOSI);
  ethernetReset(ETH_RST);
  Ethernet.init(ETH_CS);
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.println();
  Serial.println(ip);
  Serial.println(localPort);
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i = 0; i < 4; i++)
    {
      Serial.print(remote[i], DEC);
      if (i < 3)
      {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);
    disp_sens(packetBuffer);
  }
  delay(10);
}
