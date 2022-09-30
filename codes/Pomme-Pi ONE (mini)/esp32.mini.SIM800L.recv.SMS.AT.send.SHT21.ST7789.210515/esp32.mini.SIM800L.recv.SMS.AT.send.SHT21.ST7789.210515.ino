
HardwareSerial sim800l(2);

#include <TFT_eSPI.h> 
#include <SPI.h>
#include <Wire.h>  
#include "SHT21.h"
SHT21 SHT21;
#define TFT_GREY 0x5AEB // New colour

TFT_eSPI tft = TFT_eSPI(); 


float stab[4];

void get_SHT21() 
{
    SHT21.begin();
    delay(1000);
    stab[0]=SHT21.getTemperature();
    delay(100);
    stab[1]=SHT21.getHumidity();
    Serial.printf("T:%2.2f, H:%2.2f\n",stab[0],stab[1]);
}

char gnum[128],gtext[128];


void TFT_Display(char *mess)
{
char buff[128];
char *ptr; int i=0,k=2;
strcpy(buff,mess); 
Serial.println();
for(int j=0;j<12;j++)gnum[j]=mess[9+j];//strcat(gnum,"\n");
while(mess[k]!='\n') k++;
Serial.println(k);
strcpy(gtext,&mess[k+1]);strcat(gtext,"\n");
Serial.println(gtext);
Serial.println(gnum);

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10, 2);
    tft.setTextColor(TFT_RED,TFT_BLACK);  tft.setTextSize(2);
    tft.println("NUM:0781086930");
    tft.setCursor(10, 40, 2);
    tft.setTextColor(TFT_WHITE,TFT_BLACK); 
    tft.println(gnum);
    tft.setCursor(0, 80, 2);
    tft.setTextColor(TFT_BLUE,TFT_BLACK); 
    tft.println(gtext);
    tft.setCursor(120,210,1);
    tft.setTextColor(TFT_GREEN,TFT_BLACK);  tft.setTextSize(1);
    tft.println("SmartComputerLab");
}

void send_SMS(char *num,char *texte)
{
  char buff[128]; memset(buff,0x00,128);
  Serial.println("Sending SMS...");               //Show this message on serial monitor
  sim800l.print("AT+CMGF=1\r");                   //Set the module to SMS mode
  delay(100);
  strcpy(buff,"AT+CMGS=\"");  // +33
  strcat(buff,num);strncat(buff,"\"\r",2);
  Serial.println(buff);
  sim800l.print(buff);
  for(int i=0;i<24;i++) Serial.print(buff[i],HEX); Serial.println();
 // sim800l.print("AT+CMGS=\"+330682489444\"\r");  //Your phone number don't forget to include your country code, example +212123456789"
  delay(500);
  sim800l.print(texte);       //This is the text to send to the phone number, don't make it too long or you have to modify the SoftwareSerial buffer
  delay(500);
  sim800l.print((char)26);// (required according to the datasheet)
  delay(500);
  sim800l.println();
  Serial.println("Text Sent.");
  delay(500);

}

int rep=0;

void recv_SMS()
{

  Serial.println("Initializing..."); 
  delay(1000);
  rep=0;
  sim800l.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();delay(400);rep++;
  sim800l.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();delay(400);rep++;
  sim800l.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();delay(400);rep++;
}

void setup()
{
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  Wire.begin(12,14); rep=0;
  tft.init();
  tft.setRotation(1);
  //Begin serial communication with Arduino and SIM800L
  sim800l.begin(9600);
  recv_SMS();
}

void loop()
{
  updateSerial();
}

void updateSerial()
{
int i=0;char buff[128]; 
  delay(500);
  memset(buff,0x00,128);i=0;
  while (Serial.available()) 
  {
    sim800l.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  i=0;
  while(sim800l.available()) 
  {
    buff[i]=sim800l.read();
    Serial.write(buff[i]);//Forward what Software Serial received to Serial Port
    i++;
  }
 if(i && rep>2) 
   { 
    char mess[128];
    TFT_Display(buff);delay(1000);
    get_SHT21();
    delay(2000);
    sprintf(mess,"T:%2.2f, H:%2.2f",stab[0],stab[1]);
    send_SMS(gnum,mess); 
    delay(10000); recv_SMS();  
   }
}
