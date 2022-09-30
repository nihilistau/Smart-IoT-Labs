#include <LOLIN_EPD.h>
#include <Adafruit_GFX.h>
#define EPD_CS  5
#define EPD_DC 26
#define EPD_RST -1  // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY -1 // can set to -1 to not use a pin (will wait a fixed delay)

LOLIN_UC8151D EPD(212, 104, EPD_DC, EPD_RST, EPD_CS, EPD_BUSY); //hardware SPI 212,104
//LOLIN_IL3897 EPD(250, 122, EPD_DC, EPD_RST, EPD_CS, EPD_BUSY);

void setup() {
  Serial.begin(9600);
    EPD.begin();
    EPD.setRotation(0);
    EPD.clearBuffer();
    EPD.drawRect(30,30,10,10,EPD_BLACK);
    EPD.setTextSize(1);
    EPD.setTextColor(EPD_BLACK);
    EPD.setCursor(0,0);
    EPD.println("Bonjour to SmartComputerLab.");
    //EPD.display();
    //delay(4999);
    EPD.setTextSize(2);
    EPD.setTextColor(EPD_RED);
   
//    EPD.setTextSize(2);
//    EPD.setTextColor(EPD_RED);
//    EPD.setCursor(20,20);
//    EPD.println("ligne");
//    EPD.display();
}

int count=0;

void loop() {
  char ligne[64];
      EPD.begin();
      EPD.setRotation(0);
    EPD.clearBuffer();
    EPD.update();
    EPD.setCursor(20,20);
    sprintf(ligne,"count:%d\n",count); count++;
    Serial.println(count);
    EPD.println(ligne);
 
    EPD.display();
    //EPD.deepSleep();
    delay(4000);

}
