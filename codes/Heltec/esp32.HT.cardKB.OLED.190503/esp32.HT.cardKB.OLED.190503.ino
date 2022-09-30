#include <Wire.h>
#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16);

#define CARDKB_ADDR 0x5F

int val=99;
char buff[32];

char ssid[32],pass[32];
int i=0;

char cbuff[256]; int line=1;

int set_wifi(char *s)
{
 memset(s,0x00,32);
while(1)
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
        { u8x8.clear(); i=0; return(0);}
      else 
        { s[i]=c; u8x8.drawString(0,i/16+line,s+16*(i/16));i++; }
      if(i==32) 
        { u8x8.clear(); i=0; memset(s,0x00,32); return(1); }
    }
  
  }
   delay(10);
 }
}


void setup()
{

  Serial.begin(9600);
  Wire.begin(21,22);
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.clear();
  sprintf(buff,"%d", val);
  u8x8.drawString(0,0,"set WiFi");
  delay(3000);
  u8x8.drawString(0,0,"set SSID");
  set_wifi(ssid);
  Serial.println(ssid);
  u8x8.drawString(0,0,"set PASS");
  set_wifi(pass);
  Serial.println(pass);
  u8x8.clear();
}


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
        { u8x8.clear(); i=0; memset(cbuff,0x00,256); }
      else 
        { cbuff[i]=c; u8x8.drawString(0,i/16,cbuff+16*(i/16));i++; }
      if(i==256) 
        { u8x8.clear(); i=0; memset(cbuff,0x00,256); }
    }
  
  }
   delay(10);
}

