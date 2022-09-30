#include <SPI.h>              // include libraries
#include <LoRa.h>
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)
#define freq    868E6   
#define sf 8
#define sb 125E3       
#define THINGER_SERVER "90.105.150.96"
#define THINGER_PORT 25202

#define _DEBUG_
#include <WiFiClientSecure.h>
#include <ThingerESP32.h>



// Define and initialise the sensor
#define PIR 17

#define USERNAME "bako"
#define DEVICE_ID "esp32lora"
#define DEVICE_CREDENTIAL "4z5VhO6TEKfb"

#define SSID "Livebox-08B0"
#define SSID_PASSWORD "G79ji6dtEptVTPWmZP"

// define your board pin here
#define LED_PIN 25

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);
//ThingerSmartConfig thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

bool NOTIFICATION = HIGH; // "HIGH" the handle will call endpoint and hence notify user. In the future initialize from cloud
bool FRAME_DETECTED = false;
int MOTION_SENSOR = 17;
unsigned long LAST_TIME = 0;

float d1,d2;

union 
  { 
    uint8_t frame[8];
    float sensor[2]; 
  } pack;


int count=0;
int start=0;

void onReceive(int packetSize) 
{ 
  if(packetSize==8) FRAME_DETECTED=true;
}

void taskLoRa( void * parameter)
{ 
  while(1)
  {
  int i=0;
  thing.handle();
    if(FRAME_DETECTED & NOTIFICATION)
      {   
      thing.call_endpoint("LORA");
      Serial.println("Frame detected.");
      delay(1000);i=0;

      while (LoRa.available()) 
        { 
        pack.frame[i]=LoRa.read();i++; 
        } 
      FRAME_DETECTED = false;
        thing["LORA"] >> [](pson& out)
        {
        out["temp"] = pack.sensor[0];
        out["humi"] = pack.sensor[1];
        };
      }
      LoRa.receive();    
  }
}


void setup() { 
  Serial.begin(9600);
  pinMode(DI0,INPUT); 
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(sf);
  LoRa.setSignalBandwidth (sb);
  LoRa.onReceive(onReceive);  // pour indiquer la fonction ISR
  
  thing.add_wifi(SSID, SSID_PASSWORD);
  thing["NOTIFICATION"] << inputValue(NOTIFICATION,{
  if(NOTIFICATION)
    {
    Serial.println("NOTIFICATION set");
    LoRa.receive();  // pour activer l'interruption (une fois)
    }
  else { }  
  }); 
   delay(100);
   xTaskCreate(
                    taskLoRa,          /* Task function. */
                    "taskLoRa",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    0,                /* Priority of the task. */
                    NULL);             // task handle

    Serial.println("Task Lora created");

LAST_TIME = millis(); 
}


void loop()
{   
delay(6000);
}
