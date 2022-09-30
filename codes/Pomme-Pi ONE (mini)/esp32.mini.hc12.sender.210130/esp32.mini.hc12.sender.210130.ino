#define HCS 2  // HC-12 set mode
//HardwareSerial Serial1(1);  // RX-12, TX-13
HardwareSerial uart(2);  // RX-16, TX-17
int i=0,c=0; char m;
void setup()
{
Serial.begin(9600) ;
  pinMode(HCS,OUTPUT) ;  // connected to SET
  digitalWrite(HCS,LOW) ; // enter AT command mode
  uart.begin(9600, SERIAL_8N1, 16, 17);  // RXD1-17, TXD2-16
  delay(100);
  Serial.println("set channel 1");
  uart.print("AT+C001\r\n") ; // set to channel 1
  delay(100) ;
  uart.print("AT+P8\r\n") ; // set max power ­ 20dBm
  delay(100) ;
  digitalWrite(HCS,HIGH) ; // enter transparent mode
  delay(200);
  Serial.println("enter transparent mode");
}

char buff[128]="from sender";

void loop()
{
  for(i=0; i<strlen(buff);i++){ uart.write(buff[i]);delay(20); }
  delay(2000);
  Serial.println(buff);
} 
