#include "softSerial.h"
softSerial ss(GPIO5 /*TX pin*/, GPIO3 /*RX pin*/);  // 3 , 5

void setup()
{
  Serial.begin(9600);delay(200);
  Serial.println();
  Serial.println("softSerial init");
  ss.begin(9600);delay(200);
}

float s1=0.1,s2=0.2,s3=0.3,s4=0.4;

union 
{
uint8_t frame[16];
float sensor[4];
} sdp; // send data packet

void loop()
{
  Serial.println("send...");
  s1+=1.0;s2+=1.0;s3+=1.0;s4+=1.0;
  sdp.sensor[0]=s1;sdp.sensor[1]=s2;sdp.sensor[2]=s3;sdp.sensor[3]=s4;
  for(int i=0;i<16;i++) ss.sendByte(sdp.frame[i]);
  delay(1000);
}
