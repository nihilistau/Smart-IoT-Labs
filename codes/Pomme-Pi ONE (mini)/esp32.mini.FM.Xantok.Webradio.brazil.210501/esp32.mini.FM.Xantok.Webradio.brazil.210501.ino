
#include <XantoI2C.h>
#include "XantoKT0803L.h"
#include "Arduino.h"
#include "Audio.h"
#include <WiFi.h>

char *ssid = "Livebox-08B0";
char *pass = "G79ji6dtEptVTPWmZP";

#define I2S_DOUT      12 //mini // D2
#define I2S_BCLK      16 //mini // D3
#define I2S_LRC       14  //mini // 2 D1

Audio audio;

const uint8_t PIN_SCL = 2; //14;
const uint8_t PIN_SDA = 0;  //12;

XantoKT0803L fm(PIN_SCL, PIN_SDA);

char snprintf_buffer[19] = "0x00 76543210 0x00";


void TRANS_Task(void * pvParameters)
{
while(true)
  { 
   if (Serial.available()) {
    byte cmd = Serial.read();
    if (cmd == 'W') {
      String str = Serial.readString();
      int register_address, value;
      sscanf(str.c_str(), "%i %i", &register_address, &value);
      writeAndPrintRegister(register_address, value);
    } else if (cmd == 'R') {
      String str = Serial.readString();
      int register_address;
      sscanf(str.c_str(), "%i %i", &register_address);
      readAndPrintRegister(register_address);
    } else if (cmd == 'F') { 
      setFrequency(Serial.parseInt());
    } else if (cmd == 'P') {  
      readAndPrintAllRegisters();   
    } else if (cmd == 'M') {
      Serial.println("Mute");
      fm.mute(1);
      printErrorIfExists();
    } else if (cmd == 'U') {  
      Serial.println("Unmute");
      fm.mute(0);
      printErrorIfExists();   
    } else {
      Serial.println("Unknown command");
      printUsage();
    }
  }
 delay(10);
 }
}



void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);
  if(WiFi.status() != WL_CONNECTED){
  delay(500); Serial.print(".");  }

  Serial.println("WiFi connected");    
  delay(3000);
      
  Serial.println("WiFi connected");
  printUsage();
  xTaskCreatePinnedToCore(
                    TRANS_Task,   /* Function to implement the task */
                    "TRANS_Task", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    0);  /* Core where the task should run */
  Serial.println("TRANS_Task created...");
 
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(8); // 0...21
  audio.connecttohost("http://cast3.hoost.com.br:9071/live");
}

void printUsage() {
  Serial.println("Usage:");
  Serial.println(" -Write register: W register_address_hex value_hex");
  Serial.println(" -Read register: R register_address_hex");
  Serial.println(" -Set Frequency: F frequency*10");
  Serial.println(" -Print all registers: P");
  Serial.println(" -Mute: M");
  Serial.println(" -Unmute: U");
  Serial.println("E.g.: \"W 0x02 0x40\" - write value=0x40 to the register with address=0x02");
  Serial.println("E.g.: \"R 0x02\" - read the register with address=0x02");
  Serial.println("E.g.: \"F 997\" - set radio frequency=99.7MHz");
}

void printErrorIfExists() {
  if (fm.error > 0) {
    Serial.print("Error: ");
    Serial.println(fm.error);
    fm.error = 0;
  }
}

void printRegister(uint8_t register_address, uint8_t value) {
  snprintf(snprintf_buffer, 19, "0x%02X %d%d%d%d%d%d%d%d 0x%02X",
           register_address,
           bitRead(value, 7),
           bitRead(value, 6),
           bitRead(value, 5),
           bitRead(value, 4),
           bitRead(value, 3),
           bitRead(value, 2),
           bitRead(value, 1),
           bitRead(value, 0),
           value
          );
  Serial.println(snprintf_buffer);
}

void readAndPrintRegister(uint8_t register_address) {
  Serial.print("Read: ");
  Serial.println(register_address, HEX);

  uint8_t value = fm.read(register_address);
  printErrorIfExists();

  Serial.println("Register address, BIN value, HEX value:");
  printRegister(register_address, value);
}

void readAndPrintAllRegisters() {
  Serial.println("Read all: ");

  Serial.println("Register address, BIN value, HEX value:");
  for (uint8_t i = 0; i < fm.getRegistersCount(); i++) {
    uint8_t value = fm.read(fm.getRegisters()[i]);
    printErrorIfExists();
    printRegister(fm.getRegisters()[i], value);
  }
  Serial.println("Done");
}

void writeAndPrintRegister(uint8_t register_address, uint8_t value) {
  Serial.print("Write: ");
  Serial.print(register_address, HEX);
  Serial.print(" ");
  Serial.println(value, HEX);

  value = fm.write(register_address, value);
  printErrorIfExists();

  Serial.println("Register address, BIN value, HEX value:");
  printRegister(register_address, value);

  value = fm.read(register_address);
  printErrorIfExists();

  Serial.println("Register address, BIN value, HEX value:");
  printRegister(register_address, value);
}

void setFrequency(uint16_t frequency) {
  Serial.print("Set frequency: ");
  Serial.print(frequency / 10.0);
  Serial.println("MHz");
  fm.setFrequency(frequency);
}


void loop() {
      audio.loop();
}

void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}

void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}

void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}

void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}

void audio_showstreaminfo(const char *info){
    Serial.print("streaminfo  ");Serial.println(info);
}

void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}

void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}

void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}

void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}

void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}

void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}
