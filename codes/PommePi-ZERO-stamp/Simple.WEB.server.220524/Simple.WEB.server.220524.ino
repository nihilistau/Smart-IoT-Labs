#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Adafruit_NeoPixel.h>
#define PIN        2 

#define NUMPIXELS 1 // one pixel LED

const char* ssid = "PhoneAP";
const char* password = "smartcomputerlab";
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);

int t=24, h=55;

void handleRoot() { 
  char buff[32];
  sprintf(buff,"Temp:%d, Humi:%d", t,h); 
  server.send(200, "text/plain", buff);
  pixels.setPixelColor(0, pixels.Color(0, 150, 0));pixels.show(); 
  }

void handleNotFound() {
  pixels.setPixelColor(0, pixels.Color(150, 0, 0));pixels.show(); 
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  pixels.setPixelColor(0, pixels.Color(0, 150, 0));pixels.show();
}

void setup(void) {
  Serial.begin(9600);
  pixels.begin();
  WiFi.mode(WIFI_STA);
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
  pixels.setPixelColor(0, pixels.Color(0, 0, 150));pixels.show();
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
}
