#include <Wire.h>
#include <SparkFun_APDS9960.h>
const byte interruptPin = 17;

SparkFun_APDS9960 apds = SparkFun_APDS9960();

int isr_flag = 0;

void setup() 
{
  Wire.begin(12,14);  // SDA, SLC
  Serial.begin(9600);Serial.println();
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), interruptRoutine,FALLING);
// Initialize APDS-9960 (configure I2C and initial values)
  if (apds.init()) 
  {
    Serial.println("APDS-9960 initialization complete");
  } 
  else 
  {
  Serial.println("Something went wrong during APDS-9960 init!");
  }
if ( apds.enableGestureSensor(true) ) 
  {
  Serial.println("Gesture sensor is now running");
  } 
  else 
  {
  Serial.println("Something went wrong during gesture sensor init!");
  }
}

void loop() {
if( isr_flag == 1 ) // interrupt captured
  {
  detachInterrupt(0);
  delay(20);
  handleGesture();
  isr_flag = 0;
  attachInterrupt(digitalPinToInterrupt(interruptPin), interruptRoutine,FALLING);
  delay(20);
  }
}

void interruptRoutine() 
{
  isr_flag = 1;
  //Serial.println("interruption");
}

void handleGesture() 
{
  if ( apds.isGestureAvailable() ) 
  {
    switch ( apds.readGesture() ) 
      {
      case DIR_UP: Serial.println("UP"); break;
      case DIR_DOWN:Serial.println("DOWN"); break;
      case DIR_LEFT: Serial.println("LEFT"); break;
      case DIR_RIGHT: Serial.println("RIGHT"); break;
      case DIR_NEAR: Serial.println("NEAR"); break; 
      case DIR_FAR: Serial.println("FAR"); break;
      default: Serial.println("NONE");
      }
  }
}
