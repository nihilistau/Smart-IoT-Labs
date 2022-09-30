// I've written a program to run the Laser Trip Wire.
//It continually checks to see whether the button is pressed. If it is pressed we enter the setup mode, if pressed again we enter armed mode.
//If we are in setup mode it does the following:
//    It outputs the values to the serial port.
//    When the light on the photo resistor is bright enough pin 13 lights up. This feature allows one to aim the laser. 
//In the armed mode it does the following:
//    Every 3 seconds it reports via serial communications.
//    It checks if the value from the photo resistor dips due to the laser being broken, when the laser beam is broken the Trip mode is started.
//In trip mode the tripwire has been "tripped":
//    It beeps 3 times and the data is sent to the serial port,
//    then returns to the armed mode.

// Variables
int mode = 1;
int ambiant;
int trip = 1000; // The light value I get when using my laser
int minLight = 900;
int makeBeep = 1; //0 for no beep, 1 for beep!
int atAverage;
long millisCount;

// Output Pins
int laserPin = 2;
int ledPin = 13;
int buzzerPin = 3;
String  modeNames[3] = {"SETTINGS","ARMED","TRIP"};

// Input Pins
int modePin = 4; 
int tripPin = A0;
int ambiantPin = A1;


void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(laserPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(modePin, INPUT);
  Serial.begin(9600);
}


void loop() {

  // When the button is pushed
  if (digitalRead(modePin) == 1) {
    trip = analogRead(tripPin);
    mode=mode + 1;
    if (mode >= 2) {
      mode = 0;
    }
    beep(1);
    delay(300);
  }

  
  //does something when the mode changes
  switch (mode) {
    case 0: //calibration mode
      digitalWrite(laserPin,HIGH);
      
      ambiant = analogRead(ambiantPin);
      trip = analogRead(tripPin);
      atAverage = ambiant + ((trip - ambiant)/2);
      stats();
      
      if (trip  >= minLight) {
        digitalWrite(ledPin,HIGH);
      } else {
        digitalWrite(ledPin,LOW);
      }

    break;

    case 1: // Armed mode
      digitalWrite(laserPin,HIGH);
      digitalWrite(ledPin,LOW);
      ambiant = analogRead(ambiantPin);
      atAverage = ambiant + ((trip - ambiant)/2);
      if (analogRead(tripPin) < atAverage) {
        mode = 2;
      }
      if ((millis() - millisCount) >= 3000) {
        millisCount = millis();
        stats();
      }
    break;
    
    case 2: //Trip Mode
      if ((millis() - millisCount) >= 1000) {
        millisCount = millis();
        stats();
        beep(3);
        mode = 1;
      }
    break;
    }

    
  delay(1);                       // wait for a bit
}

//It Beeps
void beep(int qty) {
  int count;
  if (makeBeep == 1) {
    for (count = 1;count<=qty;count++) {
      digitalWrite(buzzerPin, HIGH);
      delay(50);
      digitalWrite(buzzerPin, LOW);
      delay(50);
    }
  }
}

//Writes stats to the Serial Port
void stats() {
  Serial.print("A:");
  Serial.print(ambiant);
  Serial.print(" T:");
  Serial.print(trip);
  Serial.print(" AT:");
  Serial.print(atAverage);
  Serial.print(" MODE:");
  Serial.print(modeNames[mode]);
  Serial.println("");
}


