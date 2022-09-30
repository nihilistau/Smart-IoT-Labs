#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>
#include <LoRa.h>

// to be defined in User_Setup.h

//#define TFT_MISO 19  // default
//#define TFT_MISO 12  
// #define TFT_MOSI 23  // default
//#define TFT_MOSI 13
//#define TFT_SCLK 18  // default
//#define TFT_SCLK 14  => 17
//#define TFT_CS    5  // default
//#define TFT_CS    15  
//#define TFT_DC    2  // Data Command control pin
//#define TFT_RST   4  // Reset pin (could connect to RST pin)

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define TFT_GREY 0x5AEB // New colour

SPIClass tft_spi(HSPI);

TFT_eSPI tft = TFT_eSPI();  // Invoke library

int rssi;

int rdcount=0;

union tspack
  {
    uint8_t frame[24];
    struct packet
      {
        uint8_t head[4];  // head[3] is length for sensors or text, text takes 1 on MSbit
        uint32_t pnum;
        float sensor[4];
      } pack;
  } rdf;  // data frame 

int sf=10,sb=125E3; 
//long freq=868500E3; 
long freq=434500E3; 


void setup(void) {
  Serial.begin(9600);
  pinMode(15,OUTPUT);
  

  Serial.println(sf);
  Serial.println(sb);
  Serial.println(freq);
  
  pinMode(26, INPUT);  // recv interrupt
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth(sb);
  LoRa.setCodingRate4(8);
  LoRa.enableCrc();

  tft_spi.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.init(tft.spi);
  delay(100);
  tft.setRotation(2);


}

void loop() {
  Serial.println("in the loop");
  // Fill screen with grey so we can see the effect of printing with and without 
  // a background colour defined
    delay(100);
  tft.fillScreen(TFT_GREY);
  // Set "cursor" at top left corner of display (0,0) and select font 2
  // (cursor will move to next line automatically during printing with  'tft.println'
  //  or stay on the line is there is room for the text with tft.print)
  tft.setCursor(0, 0, 2);
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
  // We can now plot text on screen using the "print" class
  tft.println("Hello SMTR!");
  // Set the font colour to be yellow with no background, set to font 7
  tft.setTextColor(TFT_YELLOW); tft.setTextFont(7);
  tft.println(1234.56);
  // Set the font colour to be red with black background, set to font 4
  tft.setTextColor(TFT_RED,TFT_BLACK);    tft.setTextFont(4);
  //tft.println(3735928559L, HEX); // Should print DEADBEEF
  // Set the font colour to be green with black background, set to font 4
  tft.setTextColor(TFT_GREEN,TFT_BLACK);
  tft.setTextFont(4);
  tft.println("This");
  tft.println("is your IoT,");
  // Change to font 2
  tft.setTextFont(2);
  tft.println("development platform.");
  tft.println("We hope you will have");
  tft.println("much fun,");
  // This next line is deliberately made too long for the display width to test
  // automatic text wrapping onto the next line
  tft.println("You will be able to develop almost any kind of IoT architecture");
  // Test some print formatting functions
  float fnumber = 123.45;
   // Set the font colour to be blue with no background, set to font 4
  tft.setTextColor(TFT_BLUE);    tft.setTextFont(4);
  tft.print("Float = "); tft.println(fnumber);           // Print floating point number
  tft.print("Binary = "); tft.println((int)fnumber, BIN); // Print as integer value in binary
  tft.print("Hexadecimal = "); tft.println((int)fnumber, HEX); // Print as integer number in Hexadecimal
  delay(10000);
}

