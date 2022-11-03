#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "HT_SSD1306Wire.h"
#include "Wire.h"
SSD1306Wire  display(0x3c, 500000, SDA, SCL, GEOMETRY_128_64, -1);
                    
void displayOLED(char *line1, char *line2, char *line3)
{
    
    display.init();
    display.flipScreenVertically();display.clear();
    display.drawString(20, 50, "SmartComputerLab" );
    display.drawString(0, 0,  line1 );
    display.drawString(0, 15, line2);
    display.drawString(0, 30, line3);
    display.display();
}

void setup() {
    Serial.begin(9600);
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW); 
    delay(100);
    Wire.begin(28,29);
    display.init();
    display.flipScreenVertically();
}

int c1=0,c2=0,c3=0;    

void loop()
{
char l1[32],l2[32],l3[32];
sprintf(l1,"Count1=%d",c1);sprintf(l2,"Count2=%d",c2);
sprintf(l3,"Count3=%d",c3);
c1+=1;c2+=2;c3+=3;
displayOLED(l1,l2,l3);
delay(2000);
}
