
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"


#define hSCK 17
#define hMOSI 12
#define hMISO 13
#define DC 2
#define CS 23

SPIClass hspi(HSPI);

Adafruit_ILI9341(int8_t CS, int8_t DC, int8_t hMOSI, int8_t hSCK, int8_t _RST = -1, int8_t _MISO = -1);

Adafruit_ILI9341 tft = Adafruit_ILI9341(CS, DC);

uint16_t channel_color[] = {
  ILI9341_RED,         /* 255,   0,   0 */
  ILI9341_ORANGE,      /* 255, 165,   0 */
  ILI9341_YELLOW,      /* 255, 255,   0 */
  ILI9341_GREEN,       /*   0, 255,   0 */
  ILI9341_CYAN,        /*   0, 255, 255 */
  ILI9341_MAGENTA,     /* 255,   0, 255 */
  ILI9341_RED,         /* 255,   0,   0 */
  ILI9341_ORANGE,      /* 255, 165,   0 */
  ILI9341_YELLOW,      /* 255, 255,   0 */
  ILI9341_GREEN,       /*   0, 255,   0 */
  ILI9341_CYAN,        /*   0, 255, 255 */
  ILI9341_MAGENTA,     /* 255,   0, 255 */
  ILI9341_RED,         /* 255,   0,   0 */
  ILI9341_ORANGE      /* 255, 165,   0 */
};

// char array to print to the screen
char sbuff[32];

void setup() {
  tft.begin();
  tft.setRotation(3);
  // clear the screen with a black background
   tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
  tft.setCursor(0, 0);
  tft.print(" ETN-SMTR ");
  tft.setTextColor(ILI9341_WHITE, ILI9341_GREEN);
  tft.print(" WiFi ");
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
  tft.print(" Afficheur");
  //tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 32);
}

void loop() {

  // Read the value of the sensor on A0
  int lumv = analogRead(A0);
  // set the font color
   //tft.fillScreen(ILI9341_BLACK);
     tft.setTextSize(4);
   tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
  // print the sensor value
    tft.setCursor(64, 108);
    sprintf(sbuff,"LUM:%d",lumv);
    tft.print(sbuff);
  // wait for a moment
  delay(1000);

}

