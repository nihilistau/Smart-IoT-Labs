#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

int LED_PIN = 25;

void setup()
{
Serial.begin(9600);
Serial.println("in setup");

  pinMode(LED_PIN,OUTPUT);
  pinMode(33,INPUT);
  delay(500);
  if(bootCount == 0) //Run this only the first time
  {
      digitalWrite(LED_PIN,HIGH);
      bootCount = bootCount+1;
  }
  else
  {
      digitalWrite(LED_PIN,LOW);
  } 
  delay(3000);
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //External interrupt when gpio33 goes high
esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0);
Serial.println("to sleep waiting on 1 at pin 33");
esp_deep_sleep_start();

}

void loop(){
  
}

