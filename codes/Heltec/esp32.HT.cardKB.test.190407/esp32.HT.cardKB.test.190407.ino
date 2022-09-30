#include <Wire.h>
#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16);

#define CARDKB_ADDR 0x5F

void setup()
{

  Serial.begin(9600);
  Wire.begin(21,22);
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.clear();
  u8x8.drawString(0,0,"cardKB started");
  delay(3000);
  u8x8.clear();
}

char cbuff[256];int i=0; int line=0;

void loop()
{
  Wire.requestFrom(CARDKB_ADDR, 1);
  while(Wire.available())
  {
    char c = Wire.read(); // receive a byte as characterif
    if (c != 0)
    {
      Serial.println(c, HEX);
      Serial.println(c);
      if(c=='\n' || c=='\r')  // line feed or carriage return
        { u8x8.clear(); i=0; Serial.println(cbuff);memset(cbuff,0x00,256); }
      else 
        { cbuff[i]=c; u8x8.drawString(0,i/16,cbuff+16*(i/16));i++; }
      if(i==256) 
        { u8x8.clear(); i=0; memset(cbuff,0x00,256); }
    }
  
  }
   delay(10);
}
