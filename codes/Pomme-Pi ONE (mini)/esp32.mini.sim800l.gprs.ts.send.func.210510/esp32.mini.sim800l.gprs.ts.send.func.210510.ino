
#include "GPRS_TCP.h"

void setup() {
  SIM900.begin(9600);  /* Define baud rate for software serial communication */
  Serial.begin(9600); /* Define baud rate for serial communication */
  startgpsr_TCP("free");  // define ATN in the GPRS_TCP.h
}


int count1=0,count2=0;

void loop() {
  char cbuff[128];

  for(int i=0;i<10;i++)
    {
    memset(cbuff,0x00,128);
    sprintf(cbuff,"4VZFZQSGA9L52B8X&field1=%d&field2=%d",count1,count2);
    count1++;count2+=2;
    sendgprs_TCP(cbuff);  // define IP address and port for ThingSpeak server
    }
}
