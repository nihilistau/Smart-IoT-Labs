HardwareSerial uart(0);  // RX0-20, TX0-21

void setup() {
uart.begin(9600, SERIAL_8N1, 21, 20); //RX0, Tx0
delay(1000);
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
  }
  uart.println("new");
  delay(2000);
  ptr=strstr(gpst,"$GPGGA");
  if(ptr)
    { 
      strncpy(ttime,ptr+7,6);
      ftime[0]=ttime[0];ftime[1]=ttime[1];ftime[2]=':';
      ftime[3]=ttime[2];ftime[4]=ttime[3];ftime[5]=':';
      ftime[6]=ttime[4];ftime[7]=ttime[5];
      if(ftime[1]<'9') ++ftime[1];
      else { ftime[1]='0'; ++ftime[0];}
      uart.println(ftime);
    }
    delay(1000);
}
