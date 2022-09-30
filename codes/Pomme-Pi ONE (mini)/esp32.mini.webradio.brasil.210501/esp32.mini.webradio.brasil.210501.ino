#include "Arduino.h"
#include "Audio.h"
#include <WiFi.h>

char *ssid = "Livebox-08B0";
char *pass = "G79ji6dtEptVTPWmZP";

#define I2S_DOUT      12 //mini // D2
#define I2S_BCLK      16 //mini // D3
#define I2S_LRC       14  //mini // 2 D1

Audio audio;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid,pass);
    if(WiFi.status() != WL_CONNECTED){
    delay(500); Serial.print(".");
      }

  Serial.println("WiFi connected");    
  delay(3000);
      
  Serial.println("WiFi connected");
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
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
