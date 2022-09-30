/* This code works with Sim800L and a push button
 * Press the button to send a simple SMS/Text to a specified phone number
 * Refer to www.SurtrTech.com for more details 
 */

HardwareSerial sim800l(2);

#define button1 4 // Button pin, on the other pin it's wired with GND

bool button_State; //Button state

void setup() 
{
  pinMode(button1, INPUT_PULLUP); //The button is always on HIGH level, when pressed it goes LOW
  sim800l.begin(9600);   //Module baude rate, this is on max, it depends on the version
  Serial.begin(9600);   
  delay(1000);
}
 
void loop()
{
button_State = digitalRead(button1);   //We are constantly reading the button State
 
  if (button_State == LOW) {                 // test if it's pressed
    Serial.println("Button pressed");    
    delay(200);                             //Small delay to avoid detecting the button press many times
    SendSMS("0682489444","sending text");    
   }
  if (sim800l.available()){            //Displays on the serial monitor if there's a communication from the module
    Serial.write(sim800l.read()); 
  }
}
 
void SendSMS(char *num,char *texte)
{
  char buff[128]; memset(buff,0x00,128);
  Serial.println("Sending SMS...");               //Show this message on serial monitor
  sim800l.print("AT+CMGF=1\r");                   //Set the module to SMS mode
  delay(100);
  strcpy(buff,"AT+CMGS=\"+33");
  strcat(buff,num);strncat(buff,"\"\r",2);
  Serial.println(buff);
  sim800l.print(buff);
  for(int i=0;i<24;i++) Serial.print(buff[i],HEX); Serial.println();
 // sim800l.print("AT+CMGS=\"+330682489444\"\r");  //Your phone number don't forget to include your country code, example +212123456789"
  delay(500);
  sim800l.print(texte);       //This is the text to send to the phone number, don't make it too long or you have to modify the SoftwareSerial buffer
  delay(500);
  sim800l.print((char)26);// (required according to the datasheet)
  delay(500);
  sim800l.println();
  Serial.println("Text Sent.");
  delay(500);


}
 
