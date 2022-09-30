
#include <Wire.h>  
#include "SHT21.h"
SHT21 SHT21;

#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet2.h>
#include <EthernetUdp2.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008

#define     ETH_RST        15 // 17  // 4
#define     ETH_CS          5
#define     ETH_SCLK       18
#define     ETH_MISO       19  //23
#define     ETH_MOSI       23  //19

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xAA, 0xBB };
//IPAddress ip(192, 168, 1, 178), rip(192, 168, 1, 72);
IPAddress ip(192, 168, 1, 178), rip(192, 168, 1, 172);

unsigned int localPort = 7777;      // local port to listen on
unsigned int remotePort = 8888;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged";       // a string to send back

union
{
uint8_t frame[16];
float sensor[4];
} sdp, rdp; // send and receive packets

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

float stab[8];

void get_sens()
{
    SHT21.begin();
    stab[0]=SHT21.getTemperature();
    delay(100);
    stab[1]=SHT21.getHumidity();
    Serial.printf("T:%2.2f, H:%2.2f\n",stab[0],stab[1]);
}

void setup() 
{
  Serial.begin(9600);delay(200);
  Wire.begin(12,14);delay(200);
  SPI.begin(ETH_SCLK, ETH_MISO, ETH_MOSI);
  ethernetReset(ETH_RST);
  Ethernet.init(ETH_CS);
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);


}

void loop() {
  char buff[32];
    get_sens();
    sprintf(buff,"T:%2.2f,H:%2.2f\n",stab[0],stab[1]);
   
    Udp.beginPacket(rip, remotePort);
    Udp.write(buff,strlen(buff)+1);
    Udp.endPacket();
    Serial.print(buff);Serial.print(" - packet sent to:");
    Serial.print(rip);Serial.print(":");Serial.println(remotePort);
    delay(1000);
}
