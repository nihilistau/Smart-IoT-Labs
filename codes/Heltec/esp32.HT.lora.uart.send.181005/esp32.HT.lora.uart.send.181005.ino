HardwareSerial ss(2);  

uint8_t buff[16];
int compt=0;
int len=0,ret=0;;

union 
{
  uint8_t buff[16];
  char tamp[16];
} frame;


void setup() {
  Serial.begin(9600);
  ss.begin(9600);
}

void loop() {
  compt++;
  sprintf(frame.tamp,"Packet:%d", compt);
  len=strlen(frame.tamp);
  Serial.println(len);Serial.println(frame.tamp);
  ret=ss.write(frame.buff,strlen(frame.tamp)); 
  Serial.println(ret); 
  delay(3000);  
 
}
