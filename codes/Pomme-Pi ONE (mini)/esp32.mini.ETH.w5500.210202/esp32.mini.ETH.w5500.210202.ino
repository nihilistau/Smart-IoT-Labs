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
IPAddress myip(192,168,1,99);
char cbuff[32];

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

     Udp.beginPacket(ip, 8888);
     Udp.write("acknowledged");
     Udp.endPacket();
  
 


    // give the Ethernet shield a second to initialize:
//    delay(1000);
//    Serial.print("connecting to ");
//    Serial.print(server);
//    Serial.println("...");
//
//    // if you get a connection, report back via serial:
//    if (client.connect(server, 80)) {
//        Serial.print("connected to ");
//        ipr=client.remoteIP();
//
//        Serial.println(ipr);
//        sprintf(cbuff,"%3.3u.%3.3u.%3.3u.%3.3u",ipr[0],ipr[1],ipr[2],ipr[3]);
//        // Make a HTTP request:
//        client.println("GET / HTTP/1.1");
//        //client.println("Host: www.bing.com");
//        client.println("Host: www.smartcomputerlab.org");
//        client.println("Connection: close");
//        client.println();
//    } else {
//        // if you didn't get a connection to the server:
//        Serial.println("connection failed");
//    }
//    beginMicros = micros();

}

void loop()
{
// if there are incoming bytes available
    // from the server, read them and print them:
    int len = client.available();
    if (len > 0) {
        byte buffer[80];
        if (len > 80) len = 80;
        client.read(buffer, len);
        if (printWebData) {
            Serial.write(buffer, len); // show in the serial monitor (slows some boards)
        }
        byteCount = byteCount + len;
    }

    // if the server's disconnected, stop the client:
    if (!client.connected()) {
        endMicros = micros();
        Serial.println();
        Serial.println("disconnecting.");
        client.stop();
        Serial.print("Received ");
        Serial.print(byteCount);
        sprintf(cbuff,"bytes=%d\n",byteCount);
        Serial.print(" bytes in ");
        float seconds = (float)(endMicros - beginMicros) / 1000000.0;
        Serial.print(seconds, 4);
        float rate = (float)byteCount / seconds / 1000.0;
        Serial.print(", rate = ");
        Serial.print(rate);
        Serial.print(" kbytes/second");
        Serial.println();
        sprintf(cbuff,"KB/s=%3.3f\n",rate);

        // do nothing forevermore:
        while (true) {
            delay(1);
        }
    }
}
