#include <Adafruit_GFX.h>    // Core graphics library by Adafruit
#include <Arduino_ST7789.h> // Hardware-specific library for ST7789 (with or without CS pin)
#include <SPI.h>


//#define TFT_MISO 40  // -1
#define TFT_MOSI 1
#define TFT_SCLK 6
//#define TFT_CS   40  // -1 Chip select control pin
#define TFT_DC   4 //5  // Data Command control pin
#define TFT_RST  5  //4  // Reset pin (could connect to RST pin)


//Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_CS); //for display with CS pin
Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK); //for display without CS pin


float p = 3.1415926;

void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(WHITE);
  tft.print(" seconds.");
}

void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextSize(4);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void setup(void)
{
    Serial.begin(9600);while(!Serial); Serial.println();
    //pinMode(1,OUTPUT);pinMode(4,OUTPUT);pinMode(5,OUTPUT);pinMode(6,OUTPUT);
    //digitalWrite(1,HIGH);digitalWrite(4,HIGH);digitalWrite(5,HIGH);digitalWrite(6,HIGH);
    Serial.print("Hello! ST7789 TFT Test");
    delay(200);

   tft.init(240,240);   // initialize a ST7789 chip, 240x240 pixels
    delay(200);
    tft.setRotation(2);
//    Serial.println("Initialized");
//
//    uint16_t time = millis();
//    tftPrintTest();
//    
    tft.fillScreen(BLACK);
//    time = millis() - time;
//
//    Serial.println(time, DEC);
    delay(500);
//
//    // large block of text
    tft.fillScreen(YELLOW);
//
//    // a single pixel
//    //tft.drawPixel(tft.width() / 2, tft.height() / 2, GREEN);
    delay(500);
//
   tft.fillScreen(RED);
//
//   testdrawtext("hello", GREEN);

    Serial.println("done");
    delay(1000);
    
}

void loop()
{
  testdrawtext("hello", GREEN);
  delay(200);
}
