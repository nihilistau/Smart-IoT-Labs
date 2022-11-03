#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#define PIN        2 

#define NUMPIXELS 1 // one pixel LED

const char* ssid = "RV-AP";
const char* password = NULL;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

/* Put IP Address details */
//IPAddress local_ip(192,168,1,1);
//IPAddress gateway(192,168,1,1);
//IPAddress subnet(255,255,255,0);

bool LED1status = LOW;
bool LED2status = LOW;

WebServer server(80);


void setup() {
  Serial.begin(9600);
  pixels.begin();

  WiFi.softAP(ssid, password);
  //WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
  server.on("/", handle_OnConnect);
  server.on("/ledredon", handle_ledredon);
  server.on("/ledredoff", handle_ledredoff);
  server.on("/ledgreenon", handle_ledgreenon);
  server.on("/ledgreenoff", handle_ledgreenoff);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();
  if(LED1status)
  {  pixels.setPixelColor(0, pixels.Color(150, 0, 0));pixels.show();}
  else
  {pixels.setPixelColor(0, pixels.Color(0, 0, 0));pixels.show();}
  
  if(LED2status)
  {pixels.setPixelColor(0, pixels.Color(0,150, 0));pixels.show();}
  else
  {pixels.setPixelColor(0, pixels.Color(0, 0, 0));pixels.show();}
}

void handle_OnConnect() {
  LED1status = LOW;
  LED2status = LOW;
  Serial.println("GPIO4 Status: OFF | GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status,LED2status)); 
}

void handle_ledredon() {
  LED1status = HIGH;
  Serial.println("GPIO4 Status: ON");
  server.send(200, "text/html", SendHTML(true,LED2status)); 
}

void handle_ledredoff() {
  LED1status = LOW;
  Serial.println("GPIO4 Status: OFF");
  server.send(200, "text/html", SendHTML(false,LED2status)); 
}

void handle_ledgreenon() {
  LED2status = HIGH;
  Serial.println("GPIO5 Status: ON");
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void handle_ledgreenoff() {
  LED2status = LOW;
  Serial.println("GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status,false)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t led1stat,uint8_t led2stat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>RISC-V Web Server</h1>\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  
   if(led1stat)
  {ptr +="<p>LED_RED Status: ON</p><a class=\"button button-off\" href=\"/ledredoff\">OFF</a>\n";}
  else
  {ptr +="<p>LED_RED Status: OFF</p><a class=\"button button-on\" href=\"/ledredon\">ON</a>\n";}

  if(led2stat)
  {ptr +="<p>LED_GREEN Status: ON</p><a class=\"button button-off\" href=\"/ledgreenoff\">OFF</a>\n";}
  else
  {ptr +="<p>LED_GREEN Status: OFF</p><a class=\"button button-on\" href=\"/ledgreenon\">ON</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
