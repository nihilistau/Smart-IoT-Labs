
HardwareSerial sim800l(2);

#include <TFT_eSPI.h> 
#include <SPI.h>
#include <Wire.h>
#include <Sodaq_SHT2x.h>
#define TFT_GREY 0x5AEB // New colour

TFT_eSPI tft = TFT_eSPI(); 

//void loop()
//{
//  Serial.print("Humidity(%RH): ");
//  Serial.print(SHT2x.GetHumidity());
//  Serial.print("     Temperature(C): ");
//  Serial.println(SHT2x.GetTemperature());
//  Serial.print("     Dewpoint(C): ");
//  Serial.println(SHT2x.GetDewPoint());
//  
//  delay(1000);
//}


void TFT_Display(char *mess)
{
char buff[128],dbuff[32],tbuff[32];
strcpy(buff,mess); 
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10, 2);
    tft.setTextColor(TFT_RED,TFT_BLACK);  tft.setTextSize(2);
    tft.println("NUM:0781086930");
    tft.setCursor(0, 40, 2);
    tft.setTextColor(TFT_WHITE,TFT_BLACK); 
    tft.println(buff);
    tft.setCursor(120,210,1);
    tft.setTextColor(TFT_GREEN,TFT_BLACK);  tft.setTextSize(1);
    tft.println("SmartComputerLab");
}

void setup()
{
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
    Wire.begin(12,14);
  tft.init();
  tft.setRotation(1);
  
  //Begin serial communication with Arduino and SIM800L
  sim800l.begin(9600);

  Serial.println("Initializing..."); 
  delay(1000);

  sim800l.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  
  sim800l.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  sim800l.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();
}

char buff[128]; int i=0;

void loop()
{
  updateSerial();
}

void updateSerial()
{
  delay(500);
  memset(buff,0x00,128);i=0;
  while (Serial.available()) 
  {
    sim800l.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(sim800l.available()) 
  {
    buff[i]=sim800l.read();
    Serial.write(buff[i]);//Forward what Software Serial received to Serial Port
    i++;
  }
  if(i) { TFT_Display(buff+8);delay(1000); }
}
