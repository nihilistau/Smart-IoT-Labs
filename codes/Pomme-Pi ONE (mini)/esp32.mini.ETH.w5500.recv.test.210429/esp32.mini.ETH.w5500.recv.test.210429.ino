#include    <Arduino.h>
#include    <SPI.h>
#include    <Ethernet.h>
#include <EthernetUdp.h> 

#define     ETH_RST        15 // 17  // 4
#define     ETH_CS          5
#define     ETH_SCLK       18
#define     ETH_MISO       19  //23
#define     ETH_MOSI       23  //19


uint8_t mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};




// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
IPAddress ip,ipr;  // local and remote IP
IPAddress myip(192,168,1,100), remip(192,168,1,99);
char cbuff[32];
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

EthernetUDP Udp;

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(217,160,223,214);  // numeric IP for Google (no DNS)
//char server[] = "www.google.com";    // name address for Google (using DNS)
char server[] = "www.smartcomputerlab.org";    // name address for smartcomputerlab (using DNS)

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement


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


void setup()
{
    Serial.begin(9600);

    Serial.println("ESP32 W5500 Start");

    SPI.begin(ETH_SCLK, ETH_MISO, ETH_MOSI);
    ethernetReset(ETH_RST);
    Ethernet.init(ETH_CS);
    Serial.println("Starting ETH connection...");

    Ethernet.begin(mac,myip);

//    if (Ethernet.begin(mac) == 0) {
//       Serial.println("Failed to configure Eth using DHCP");
//
//
//        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
//          Serial.println("Eth shield was not found.");
//
//        } else if (Ethernet.linkStatus() == LinkOFF) {
//          Serial.println("Eth cable is not connected.");
//
//        }
//        // no point in carrying on, so do nothing forevermore:
//        while (true) {
//            delay(1);
//        }
//    }


    Serial.print("Ethernet IP is: ");
    ip = Ethernet.localIP();memset(cbuff,0x00,32);
    sprintf(cbuff,"%3.3u.%3.3u.%3.3u.%3.3u",ip[0],ip[1],ip[2],ip[3]);
    Serial.println(ip);
    Udp.begin(8888);
delay(1000);
}

void loop()
{
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
    }
 delay(10);
}
