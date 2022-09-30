#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

#define OV2640_CHIPID_HIGH   0x0A
#define OV2640_CHIPID_LOW 0x0B

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

// Enabe debug tracing to Serial port.
#define DEBUGGING
// Here we define a maximum framelength to 64 bytes. Default is 256.
#define MAX_FRAME_LENGTH 64
// Define how many callback functions you have. Default is 1.
#define CALLBACK_FUNCTIONS 1
// set GPIO16 as the slave select :
const int CS = 18;
int wifiType = 0; // 0:Station  1:AP
//const char* ssid = "Livebox­28B7"; // Put your SSID here
//const char* password = "tonbridge"; // Put your PASSWORD here
const char* ssid = "Livebox-08B0"; // Put your SSID here
const char* password = "G79ji6dtEptVTPWmZP"; // Put your PASSWORD here
IPAddress ip(192, 168, 1, 81);  // to work with RPI3 gateway
IPAddress gw(192, 168, 1, 1);  // to work with RPI3 gateway
IPAddress sn(255, 255, 255, 0);  // to work with RPI3 gateway
WebServer server(80);

ArduCAM myCAM(OV2640, 18);
void start_capture(){
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
}
void camCapture(ArduCAM myCAM){
  WiFiClient client = server.client();
  
  size_t len = myCAM.read_fifo_length();
  if (len >= 393216){
    Serial.println("Over size.");
    return;
  }else if (len == 0 ){
    Serial.println("Size is 0.");
    return;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  SPI.transfer(0xFF);
  
  if (!client.connected()) return;
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content­Type: image/jpeg\r\n";
  response += "Content­Length: " + String(len) + "\r\n\r\n";
  server.sendContent(response);
  static const size_t bufferSize = 1024;  //4096;
  static uint8_t buffer[bufferSize] = {0xFF};
  while (len) {
      size_t will_copy = (len < bufferSize) ? len : bufferSize;
      SPI.transferBytes(&buffer[0], &buffer[0], will_copy);
      if (!client.connected()) break;
      client.write(&buffer[0], will_copy);
      len-=will_copy;
  }
  myCAM.CS_HIGH();
}
void serverCapture(){
  start_capture();
  Serial.println("CAM Capturing");
  int total_time = 0;
  total_time = millis();
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  total_time = millis()-total_time;
  Serial.print("capture total_time used (in miliseconds):");
  Serial.println(total_time, DEC);
  total_time = 0;
  Serial.println("CAM Capture Done!");
  total_time = millis();
  camCapture(myCAM);
  total_time = millis()-total_time;
  Serial.print("send total_time used (in miliseconds):");
  Serial.println(total_time, DEC);
  Serial.println("CAM send Done!");
}
void resolution(){
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.send(200, "text/plain", message);
  
  if (server.hasArg("ql")){
    int ql = server.arg("ql").toInt();
    myCAM.OV2640_set_JPEG_size(ql);
    Serial.println("QL change to: " + server.arg("ql"));
  }
}
void setup() {
  uint8_t vid, pid;
  uint8_t temp;
  Wire.begin();
  Serial.begin(9600);
  Serial.println("ArduCAM Start!");
  // set the CS as an output:
  pinMode(SS, OUTPUT);
  // initialize SPI:
  
  SPI.begin(SCK,MISO,MOSI,SS);
  SPI.setFrequency(4000000); // default 4MHz
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55){
    Serial.println("SPI1 interface Error!");
    while(1);
  }
  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
   if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 )))
    Serial.println("Can't find OV2640 module!");
    else
    Serial.println("OV2640 detected.");
  //Change to JPEG capture mode and initialize the OV2640 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_640x480);
  myCAM.clear_fifo_flag();
  //myCAM.write_reg(ARDUCHIP_FRAMES, 0x00);
  if (wifiType == 0){
    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    WiFi.config(ip,gw,sn);  // to fix IP address related to RPI3 softAP
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("WiFi connected");
    Serial.println("");
    Serial.println(WiFi.localIP());
  }else if (wifiType == 1){
    Serial.println();
    Serial.println();
    Serial.print("Share AP: ");
    Serial.println(ssid);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.println("");
    Serial.println(WiFi.softAPIP());
  }
  server.on("/capture", HTTP_GET, serverCapture);
  server.on("/config", HTTP_GET, resolution);
  //server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Server started");
}
void loop() {
  server.handleClient();
}

  
  

