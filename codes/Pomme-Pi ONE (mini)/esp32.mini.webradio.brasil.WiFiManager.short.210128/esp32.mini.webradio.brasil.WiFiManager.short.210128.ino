#include "Arduino.h"
#include "Audio.h"
#include <WiFiManager.h>
// pin configuration for I2S bus – may be modified
#define I2S_DOUT      12 //mini
#define I2S_BCLK      16 //mini - 17 wemos
#define I2S_LRC       14  //mini
#define buttonPin     0   //

Audio audio;
WiFiManager wm;

const char* ssid = "ESP32";
const char* password = "smartcomputerlab";

int count=10;

void setup() {
int ret=0,sta=0;
Serial.begin(9600);
pinMode(0,INPUT_PULLUP); 
  WiFi.begin();
 //Wait for WiFi to connect to AP
  Serial.println("Waiting for WiFi");
  while (WiFi.status()!= WL_CONNECTED && count>0) {
    delay(500); count--;
    Serial.print(count);
  }
    if(!count)
    {
    if(!wm.autoConnect(ssid, password))
    Serial.println("Erreur de connexion.");
  else
    Serial.println("Connexion etablie!");
    delay(1000);
    Serial.println("");
    //Wait for WiFi to connect to AP
    Serial.println("Waiting for WiFi");
        while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
  }
  Serial.println("WiFi Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(6); // 0...21
  audio.connecttohost("http://cast3.hoost.com.br:9071/live");
    //audio.connecttohost("http://cast3.hoost.com.br:9071/live");
}

void loop()
{
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

    
