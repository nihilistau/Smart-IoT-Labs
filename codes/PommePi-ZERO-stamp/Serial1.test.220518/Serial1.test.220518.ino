
#define RX 4 //18 //18
#define TX 5 //19 //19

void setup() 
{
   Serial.begin(9600);
   //pinMode(RX,INPUT);pinMode(TX,OUTPUT);
   Serial1.begin(9600, RX,TX); //int8_t rxPin=4, int8_t txPin=5 
}

void loop() {
    if (Serial.available()) {      // If anything comes in Serial (USB),
    Serial1.write(Serial.read());   // read it and send it out Serial1 (pins 4 & 5)
  }

  if (Serial1.available()) {     // If anything comes in Serial1 (pins 4 & 5)
    Serial.write(Serial1.read());   // read it and send it out Serial (USB)
  }
}
