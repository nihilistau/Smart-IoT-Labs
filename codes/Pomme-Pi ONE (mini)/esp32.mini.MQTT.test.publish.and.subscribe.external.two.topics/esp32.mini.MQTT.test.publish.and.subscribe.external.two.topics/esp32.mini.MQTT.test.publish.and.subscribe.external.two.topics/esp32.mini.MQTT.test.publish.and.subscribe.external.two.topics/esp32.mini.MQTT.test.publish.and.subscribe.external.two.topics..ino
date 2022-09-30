#include <WiFi.h>
#include <MQTT.h>

#define NT  4  // max number of terminals
#define NTS 4  // max number of sensors per terminal

const char* ssid = "Livebox-08B0";
const char* pass = "G79ji6dtEptVTPWmZP";


const char* mqttServer = "broker.emqx.io";


WiFiClient net;
MQTTClient client;
unsigned long lastMillis = 0;

void connect() {
  char cbuff[128];
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nconnecting...");
  while (!client.connect("IoT.GW1")) {
    Serial.print("."); delay(1000);
  }
  Serial.println("\nIoT.GW1 - connected!");
  for(int i=0; i<NT ; i++)
    for(int j=0; j<NTS; j++)
      {
      sprintf(cbuff,"/esp32_GW1/Term%d/Sens%d",i,j); 
      Serial.println(cbuff); 
      client.subscribe(cbuff);
      delay(1000);
      }
  
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  // send LoRa message depending on topic
}


void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  client.begin(mqttServer, net);
  client.onMessage(messageReceived);
  connect();
}

void loop() {
  int tn,tsn, val; char cbuff[128], vbuff[15];
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability
  if (!client.connected()) { connect(); }
  if (millis() - lastMillis > 20000) {   // publish a message every 10 seconds
    lastMillis = millis();
    tn=random(0,NT); tsn=random(0,NTS); val=random(0,1000);
    sprintf(cbuff,"/esp32_GW1/Term%d/Sens%d",tn,tsn);
    sprintf(vbuff,"%d",val);
    client.publish(cbuff, vbuff);
    delay(4000);

  }
}
