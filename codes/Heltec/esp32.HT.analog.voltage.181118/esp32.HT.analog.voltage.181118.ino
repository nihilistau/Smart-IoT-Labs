int analogPin = 36;  
 
void setup()
{
  Serial.begin(9600);
}
 
void loop()
{
  int adcVal = analogRead(analogPin);
  float voltage = adcVal*(3.3/4095.0);
 
  if(voltage == 0)
  {
    Serial.println("A problem has occurred with the sensor.");
  }
  else if(voltage < 0.1)
  {
    Serial.println("Pre-heating the sensor...");
  }
  else
  {
 
    float voltageDiference=voltage-0.4;
    float concentration=(voltageDiference*5000.0)/1.6;
 
    Serial.print("voltage:");
    Serial.print(voltage);
    Serial.println("V");
 
    Serial.print(concentration);
    Serial.println("ppm");
  }
 
  delay(2000); 
 
}

