HardwareSerial SIM800L(2);

#define TINY_GSM_MODEM_SIM800
#define SerialAT SIM800L
// Your GPRS credentials, if any
const char apn[] = "free";
const char gprsUser[] = "";
const char gprsPass[] = "";

const char server[] = "86.217.14.104";  // ThingSpeak server IP
const int  port = 443;      // ThingSpeak server port
//const char resource[] ="/channels/174/fields/1/last?key=V7M6I771U8OJ2JSV";
const char resource[] ="/update?key=4VZFZQSGA9L52B8X&field1=43.91";

#include <TinyGsmClient.h>

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

void setup() {
  // Set console baud rate
  Serial.begin(9600);
  delay(10);
  Serial.println("Wait...");
  SerialAT.begin(9600);
  delay(6000);
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();
  // modem.init();

  String modemInfo = modem.getModemInfo();
  Serial.print("Modem Info: ");
  Serial.println(modemInfo);

  Serial.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" success");

  if (modem.isNetworkConnected()) {
    Serial.println("Network connected");
  }

  // GPRS connection parameters are usually set after network registration
    Serial.print("Connecting to ");
    Serial.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      Serial.println(" fail");
      delay(10000);
      return;
    }
    Serial.println(" success");

  if (modem.isGprsConnected()) {
    Serial.println("GPRS connected");
  }

  Serial.print("Connecting to ");
  Serial.println(server);
  if (!client.connect(server, port)) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" success");
}

  void loop()
  {
  for(int i=0;i<4;i++)
    {
    // Make a HTTP GET request:
    delay(6000);
    Serial.println("Performing HTTP GET request...");
    //client.print(String("GET ") + "/channels/174/fields/1/last?key=V7M6I771U8OJ2JSV" + " HTTP/1.1\r\n");
    client.print(String("GET ") + resource + " HTTP/1.1\r\n");
    client.print(String("Host: ") + server + "\r\n");
    //client.print("Connection: close\r\n\r\n");  // to keep connection alive
    client.println();

    uint32_t timeout = millis();
    while (client.connected() && millis() - timeout < 10000L)
      {
      // Print available data
      while (client.available()) {
        char c = client.read();
        Serial.print(c);
        timeout = millis();
       }
      }
    Serial.println();
    }

  // Shutdown
  client.stop();
  Serial.println("Server disconnected");
  modem.gprsDisconnect();
  Serial.println("GPRS disconnected");
  // Do nothing forevermore
  while (true) {
    delay(1000);
  }
}
