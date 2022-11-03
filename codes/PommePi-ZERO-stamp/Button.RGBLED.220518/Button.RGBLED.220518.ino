
#include <Adafruit_NeoPixel.h>
#define PIN        2 
#define NUMPIXELS  1 
#define BUT  3

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

void setup() {
  Serial.begin(9600);
  // initialize the pushbutton pin as an input:
  pinMode(BUT, INPUT_PULLUP);
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear(); // Set all pixel colors to 'off'
}
void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(BUT);
  // Show the state of pushbutton on serial monitor
  Serial.println(buttonState);
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == 1) {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.show(); 
    } else {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    pixels.show(); 
  }
  // Added the delay so that we can see the output of button
  delay(100);
}
