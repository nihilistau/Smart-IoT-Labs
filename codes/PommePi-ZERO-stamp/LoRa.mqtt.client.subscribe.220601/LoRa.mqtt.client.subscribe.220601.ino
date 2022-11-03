#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SPI.h>               
#include <LoRa.h>
#define SCK     6   // GPIO18 -- SX127x's SCK
#define MISO    0 //7   // GPIO19 -- SX127x's MISO
#define MOSI    1 //8   // GPIO23 -- SX127x's MOSI
#define SS      10   // GPIO05 -- SX127x's CS
#define RST     4   // GPIO15 -- SX127x's RESET
#define DI0     5   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define freq    434E6   
#define sf 7
#define sb 125E3 
// Replace the next variables with your SSID/Password combination
const char *ssid     = "PhoneAP";
const char *pass = "smartcomputerlab";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.43.243"; // "YOUR_MQTT_BROKER_IP_ADDRESS";
const char* mqtt_server = "broker.emqx.io";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
char *topic="risc-v/test";

typedef union
{
uint8_t  buff[64];   // total data –64 bytes
struct 
  {
  char topic[32];  // MQTT topic
  char mess[32];   // MQTT message
  } data;
} mqttpack_t;

mqttpack_t spack;



void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_LoRa() 
{                  
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  Serial.println();delay(100);Serial.println();
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
Serial.println("Starting LoRa OK!");
LoRa.setSpreadingFactor(sf);
LoRa.setSignalBandwidth(sb);
}

void callback(char* topic, byte* message, unsigned int length) 
{
  char mess[32];
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  for (int i = 0; i < length; i++) 
    {
    mess[i]=(char)message[i];
    Serial.print((char)message[i]);
    }
  Serial.println();
  LoRa.beginPacket();                   // start packet
  strcpy(spack.data.mess,mess); strcpy(spack.data.topic,topic);
  LoRa.write(spack.buff,64);
  LoRa.endPacket();
  Serial.println("Packet sent");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("PommePiClient")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(topic);
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setup_LoRa();delay(400);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

int counter=0;
char buff[64];

void loop() 
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
