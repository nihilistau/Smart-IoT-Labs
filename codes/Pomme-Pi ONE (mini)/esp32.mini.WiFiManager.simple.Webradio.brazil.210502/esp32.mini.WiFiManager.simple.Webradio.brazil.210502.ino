
#include <XantoI2C.h>
#include "XantoKT0803L.h"
#include "Arduino.h"
#include "Audio.h"
#include <ESP_WiFiManager.h> 

//char *ssid = "Livebox-08B0";
//char *pass = "G79ji6dtEptVTPWmZP";

char *ssid = "PhoneAP";
char *pass = "smartcomputerlab";



#define I2S_DOUT      21  //12 //mini // D2
#define I2S_BCLK      17  //16 //mini // D3
#define I2S_LRC       22  //14  //mini // 2 D1

Audio audio;

const uint8_t PIN_SCL = 14; //2; //14;
const uint8_t PIN_SDA = 12; //0;  //12;

XantoKT0803L fm(PIN_SCL, PIN_SDA);

char snprintf_buffer[19] = "0x00 76543210 0x00";


void setFrequency(uint16_t frequency) {
  Serial.print("Set frequency: ");
  Serial.print(frequency / 10.0);
  Serial.println("MHz");
  fm.setFrequency(frequency);
}

uint16_t freq=992;


void setup() {
  Serial.begin(9600);
  Serial.print(F("\nStarting AutoConnect_ESP32_minimal on ")); 
  Serial.println(ARDUINO_BOARD); 
  Serial.println(ESP_WIFIMANAGER_VERSION);
  ESP_WiFiManager ESP_wifiManager("AutoConnectAP");
  ESP_wifiManager.autoConnect("AutoConnectAP");
  if (WiFi.status() == WL_CONNECTED) { Serial.print(F("Connected. Local IP: "));Serial.println(WiFi.localIP()); }
  else { Serial.println(ESP_wifiManager.getStatus(WiFi.status())); }

  Serial.println("WiFi connected");    
  delay(3000);
      
  setFrequency(freq);  // use I2C bus for control

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);   // reuse I2C bus PCM signal transfer
  audio.setVolume(8); // 0...21
  audio.connecttohost("http://cast3.hoost.com.br:9071/live");
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
