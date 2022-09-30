byte ledPin = 22;
byte interruptPin = 0;  // touch pin
/* hold the state of LED when toggling */
volatile byte state = LOW;

void setup() {
  pinMode(ledPin, OUTPUT);
  /* set the interrupt pin as input pullup*/
  pinMode(interruptPin, INPUT_PULLUP);
  /* attach interrupt to the pin
  function blink will be invoked when interrupt occurs
  interrupt occurs whenever the pin change value */
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, CHANGE);
}

void loop() {
}
/* interrupt function toggle the LED */
void blink() {
  state = !state;
  digitalWrite(ledPin, state);
}
