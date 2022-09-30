HardwareSerial SIM800L(2);

#define TINY_GSM_MODEM_SIM800
#define SerialAT SIM800L

//#define TINY_GSM_DEBUG Serial

// Your GPRS credentials, if any
const char apn[] = "free";
const char gprsUser[] = "";
const char gprsPass[] = "";

#include <TinyGsmClient.h>
#include "ThingSpeak.h" // veryfy the IP address and port number in this file

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
 
#define LED_PIN 22
int ledStatus = LOW;


void setup() {
  Serial.begin(9600);
  delay(10);

  pinMode(LED_PIN, OUTPUT);
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

while(!ThingSpeak.begin(client)) 
  { Serial.println("Wait to connect to ThingSpeak"); delay(1000);} // Initialize ThingSpeak
}

void loop() {
float temp=23.7, humi=67.76;  
ThingSpeak.setField(1,temp);
ThingSpeak.setField(2,humi);
int x=ThingSpeak.writeFields((uint32_t)174,"4VZFZQSGA9L52B8X");
Serial.printf("ThingSpeak return code: %d\n",x);
delay(10000);
}
