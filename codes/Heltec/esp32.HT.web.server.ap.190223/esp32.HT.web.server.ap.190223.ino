
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
WebServer server(80);

const char* ssid = "smartcomputerlab";
const char* password = "esp32";

const int led = 25;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp32!");
  digitalWrite(led, 0);
}

void handleON() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "LED set ON");
}

void handleOFF() {
  digitalWrite(led, 0);
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
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(9600);

  
WiFi.mode( WIFI_AP );
IPAddress ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet( 255, 255, 255, 0 );
WiFi.softAPConfig( ip, gateway, subnet );
WiFi.softAP( "smartcomputerlab" );

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());


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
