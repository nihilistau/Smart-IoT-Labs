HardwareSerial ss(2);  

char buff[16];
int i=0;

void setup() {
  Serial.begin(9600);
  ss.begin(9600);
}

void loop() 
{
  i=0;
   if(ss.available() > 1){//Read from UM402 and send to serial monitor
    String input = ss.readString();
   Serial.println(input); 
   }
  delay(20);
}

