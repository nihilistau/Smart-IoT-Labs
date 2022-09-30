
int led = 22;
void setup() 
{
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  pinMode(16, INPUT);
}

void loop() {
  int data = digitalRead(16); // 
  Serial.println(data);
  digitalWrite(led, !data); // 
  delay(100);
}
