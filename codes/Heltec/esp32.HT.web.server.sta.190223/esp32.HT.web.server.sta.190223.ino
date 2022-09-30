
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
WebServer server(80);

const char* ssid = "Livebox-08B0";
const char* password = "G79ji6dtEptVTPWmZP";

const int led = 25;
const int relay= 22;  // D1 on mini shield

void handleRoot() {
  //digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp32!");
  //digitalWrite(led, 0);
}

void handleON() {
  //digitalWrite(led, 1);  //digitalWrite(relay, HIGH);
  server.send(200, "text/plain", "LED set ON");
}

void handleOFF() {
 // digitalWrite(led, 0);  //digitalWrite(relay, LOW);
  server.send(200, "text/plain", "LED set OFF");
}


void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void){
//  pinMode(led, OUTPUT);
//  pinMode(relay, OUTPUT);
//  digitalWrite(led, 0);
  Serial.begin(9600);
  WiFi.disconnect(true);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/on", handleON);
  server.on("/off", handleOFF);
  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}
