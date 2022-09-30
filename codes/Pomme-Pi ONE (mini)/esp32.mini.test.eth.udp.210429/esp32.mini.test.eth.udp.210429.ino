#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h> 

#define     ETH_RST        15 // 17  // 4
#define     ETH_CS          5
#define     ETH_SCLK       18
#define     ETH_MISO       19  //23
#define     ETH_MOSI       23  //19


uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAD};
IPAddress ip,ipr;  // local and remote IP
char cbuff[32];
unsigned int localPort = 8888, remotePort=8888;      // local port to listen on
// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged";       // a string to send back
// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void ethernetReset(const uint8_t resetPin)
{
pinMode(resetPin, OUTPUT);
digitalWrite(resetPin, HIGH); delay(250);
digitalWrite(resetPin, LOW); delay(50);
digitalWrite(resetPin, HIGH); delay(350);
}

void setup() 
{
 Serial.begin(9600);

    Serial.println("ESP32 W5500 Start");

    SPI.begin(ETH_SCLK, ETH_MISO, ETH_MOSI);
    ethernetReset(ETH_RST);
    Ethernet.init(ETH_CS);
    Serial.println("Starting ETH connection...");

    if (Ethernet.begin(mac) == 0) {
       Serial.println("Failed to configure Eth using DHCP");


        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
          Serial.println("Eth shield was not found.");

        } else if (Ethernet.linkStatus() == LinkOFF) {
          Serial.println("Eth cable is not connected.");

        }
        // no point in carrying on, so do nothing forevermore:
        while (true) {
            delay(1);
        }
    }


    Serial.print("Ethernet IP is: ");
    ip = Ethernet.localIP();memset(cbuff,0x00,32);
    sprintf(cbuff,"%3.3u.%3.3u.%3.3u.%3.3u",ip[0],ip[1],ip[2],ip[3]);
    Serial.println(ip);
    {
    Serial.println("Failed to configure Eth using DHCP");
    if(Ethernet.hardwareStatus() == EthernetNoHardware) 
      {
      Serial.println("Eth shield was not found.");
       } 
    else if (Ethernet.linkStatus() == LinkOFF) 
      {
      Serial.println("Eth cable is not connected.");
      }
    // no point in carrying on, so do nothing forevermore:
    while (true) delay(1);
    }

      Ethernet.begin(mac,ip);
  Serial.print("Ethernet IP is: ");
  ip = Ethernet.localIP();memset(cbuff,0x00,32);
  sprintf(cbuff,"%3.3u.%3.3u.%3.3u.%3.3u",ip[0],ip[1],ip[2],ip[3]);
  Serial.println(ip);
  
  Udp.begin(localPort);

}
void loop() {
    // if there's data available, read a packet
    int packetSize = Udp.parsePacket();
    if (packetSize) {
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remote = Udp.remoteIP();
        for (int i = 0; i < 4; i++) {
            Serial.print(remote[i], DEC);
            if (i < 3) {
                Serial.print(".");
            }
        }
        Serial.print(", port ");
        Serial.println(Udp.remotePort());
        // read the packet into packetBufffer
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
        Serial.println("Contents:");
        Serial.println(packetBuffer);
        // send a reply to the IP address and port that sent us the packet we received
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(ReplyBuffer);
        Udp.endPacket();
    }
    delay(10);
}
