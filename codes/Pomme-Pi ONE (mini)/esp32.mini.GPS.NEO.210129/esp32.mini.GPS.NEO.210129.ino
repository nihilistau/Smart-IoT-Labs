// The serial connection to the GPS module
HardwareSerial uart(2);

void setup(){
  Serial.begin(9600);
  uart.begin(9600, SERIAL_8N1, 17, 16);  // RXD1-17, TXD2-16
}

int i=0,ihour;
char gpst[2048],ttime[16],ftime[16],hour[8],nhour[2];
char *ptr=NULL;

void loop(){
  i=0;memset(gpst,0x00,2048);
  while (uart.available() > 0){
    // get the byte data from the GPS
    byte gpsData = uart.read();
    gpst[i]=gpsData;i++;
    Serial.write(gpsData);
  }
  ptr=strstr(gpst,"$GPGGA");
  if(ptr)
    { 
      strncpy(ttime,ptr+7,6);
      //Serial.println(ptr);
      //Serial.println(ttime);
      ftime[0]=ttime[0];ftime[1]=ttime[1];ftime[2]=':';
      ftime[3]=ttime[2];ftime[4]=ttime[3];ftime[5]=':';
      ftime[6]=ttime[4];ftime[7]=ttime[5];
      if(ftime[1]<'9') ++ftime[1];
      else { ftime[1]='0'; ++ftime[0];}
      Serial.println(ftime);
    }
}
