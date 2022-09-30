//#define THINGER_SERVER "192.168.1.49"
#define THINGER_SERVER "90.105.150.96"
//#define THINGER_PORT 80
#define THINGER_PORT 25202

#define _DEBUG_

#include <WiFiClientSecure.h>
#include <ThingerESP32.h>



// Define and initialise the sensor
#define PIR 17

#define USERNAME "smtr"
#define DEVICE_ID "esp32PIR"
#define DEVICE_CREDENTIAL "cow#1Hz7ngyq"

#define SSID "Livebox-08B0"
#define SSID_PASSWORD "G79ji6dtEptVTPWmZP"

// define your board pin here
#define LED_PIN 25

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);
//ThingerSmartConfig thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

bool NOTIFICATION = HIGH; // "HIGH" the handle will call endpoint and hence notify user. In the future initialize from cloud
bool MOTION_DETECTED = false;
int MOTION_SENSOR = 17;
unsigned long LAST_TIME = 0;




void pinChanged() {
  MOTION_DETECTED = true;
}



void setup() { 
  Serial.begin(9600);
  pinMode(MOTION_SENSOR, INPUT);
  attachInterrupt(MOTION_SENSOR, pinChanged, RISING);
  thing.add_wifi(SSID, SSID_PASSWORD);
  thing["NOTIFICATION"] << inputValue(NOTIFICATION,{
     /* here you could even attach/detach the interruption based on the updated NOTIFICATION
        value, turn off the PIR sensor, etc., but it is not really necessary */
        if (!NOTIFICATION) {
          detachInterrupt(MOTION_SENSOR);
        }
        else {
          attachInterrupt(MOTION_SENSOR, pinChanged, RISING);
        }
  });

LAST_TIME = millis(); 
     
}

int count=0;

void loop()
{
  int i=0;
  thing.handle();
    if(MOTION_DETECTED & NOTIFICATION)
      {
      thing.call_endpoint("MOTION_SENSOR");
      Serial.println("Motion detected.");
      delay(1000);count++;
      MOTION_DETECTED = false;
        thing["PIR"] >> [](pson& out)
        {
        out["pir"] = count;
        };
      }   
}
