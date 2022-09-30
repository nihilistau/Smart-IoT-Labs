

// Include the SparkFun TMP102 library.
// Click here to get the library: http://librarymanager/All#SparkFun_TMP102

#include <Wire.h> // Used to establied serial communication on the I2C bus
#include <SparkFunTMP102.h> // Used to send and recieve specific information from our sensor

const int ALERT_PIN = A3;

uint16_t timeout=10;

TMP102 sensor0;


void setup() {
  Serial.begin(9600);
  Wire.begin(12,14); //Join I2C Bus
  
  pinMode(ALERT_PIN,INPUT);  // Declare alertPin as an input
  
  if(!sensor0.begin())
  {
    Serial.println("Cannot connect to TMP102.");
    Serial.println("Is the board connected? Is the device ID correct?");
    while(1);
  }
  
  Serial.println("Connected to TMP102!");
  delay(100);

  
  // set the Conversion Rate (how quickly the sensor gets a new reading)
  //0-3: 0:0.25Hz, 1:1Hz, 2:4Hz, 3:8Hz
  sensor0.setConversionRate(2);
  
  //set Extended Mode.
  //0:12-bit Temperature(-55C to +128C) 1:13-bit Temperature(-55C to +150C)
  sensor0.setExtendedMode(0);

  float temperature;
  // Turn sensor on to start temperature measurement.
  // Current consumtion typically ~10uA.
  sensor0.wakeup();

  temperature = sensor0.readTempC();
  
  // Place sensor in sleep mode to save power.
  // Current consumtion typically <0.5uA.

  // Print temperature and alarm state
  Serial.print("Temperature: ");
  Serial.println(temperature);
esp_sleep_enable_timer_wakeup(1000*1000*timeout);  // in micro-seconds - timeout in seconds

esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);

  Serial.println("Going to sleep now");
  Serial.flush(); delay(60); 
  sensor0.sleep();
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}
 
void loop()
{
  
}
