#include <SoftwareSerial.h>

SoftwareSerial mySerial;


void setup()
{
  Serial.begin(9600); delay(200); 
  mySerial.begin(9600,SWSERIAL_8N1,4,5,false, 95, 11);  //SERIAL_8N1, 8 data bits, 1 stop bit, no parity and no flow control
  Serial.println("Software serial start");
  delay(2000);
}

int i=0,ihour;
char ttime[16],ftime[16],hour[8],nhour[2];
byte gpst[2048];
char *ptr=NULL;

void loop(){
  i=0;memset(gpst,0x00,2048);
  while (mySerial.available() > 0){
    // get the byte data from the GPS
    char gpsData = mySerial.read();
    gpst[i]=gpsData;i++;
    Serial.print(gpsData);Serial.print(' ');
  }

  Serial.println(i);
//  Serial.println("new");
//  ptr=strstr(gpst,"$GPGGA");
//  if(ptr)
//    { 
//      strncpy(ttime,ptr+7,6);
//      //Serial.println(ptr);
//      //Serial.println(ttime);
//      ftime[0]=ttime[0];ftime[1]=ttime[1];ftime[2]=':';
//      ftime[3]=ttime[2];ftime[4]=ttime[3];ftime[5]=':';
//      ftime[6]=ttime[4];ftime[7]=ttime[5];
//      if(ftime[1]<'9') ++ftime[1];
//      else { ftime[1]='0'; ++ftime[0];}
//      Serial.println(ftime);
//    }
    delay(500);
}
