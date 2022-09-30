#include <WifiEspNow.h>
#include <WiFi.h>

#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock,


//static uint8_t PEER[] {0x3C, 0x71, 0xBF, 0x97, 0xDC, 0x01}; // ESP32.HT.simple
//static uint8_t PEER[] {0x30, 0xAE, 0xA4, 0x4C, 0x57, 0x1D}; // ESP32.HT.board
static uint8_t PEER[] {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // ESP32.HT.board

QueueHandle_t oledqueue;
int queueSize=4;

void taskOLED( void * parameter)
{ 
  const TickType_t xDelay = 4000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
  uint8_t dbuff[64]; char obuff[64];
  
  while(true)
  {
  xQueueReceive(oledqueue,dbuff,10000);
  u8x8.clear();
  strncpy(obuff,(char *)dbuff,16);
  u8x8.drawString(0,0,(char*)obuff);
  strncpy(obuff,(char *)(dbuff+16),16);
  u8x8.drawString(0,1,(char*)obuff);
  vTaskDelay(xDelay);
  }
} 

char abuff[64];
void printReceivedMessage(const uint8_t mac[6], const uint8_t* dbuff, size_t count, void* cbarg) 
{
  char buff[64];
  Serial.printf("Message from %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  sprintf(abuff,"MAC:%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(((char*)dbuff));
  Serial.println();strcpy(buff,abuff);
  strcat(buff,(char*)dbuff);
  xQueueReset(oledqueue);
  xQueueSend(oledqueue, buff, portMAX_DELAY);
}

void setup() {
  Serial.begin(9600);
  Serial.println();
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.clear();

  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESPNOW", nullptr, 3);
  WiFi.softAPdisconnect(false);

  Serial.print("MAC address of this node is ");
  Serial.println(WiFi.softAPmacAddress());

  bool ok = WifiEspNow.begin();
  if (!ok) {
    Serial.println("WifiEspNow.begin() failed");
    ESP.restart();
  }
  else Serial.println("WifiEspNow.begin() OK");

  WifiEspNow.onReceive(printReceivedMessage, nullptr);

  
oledqueue = xQueueCreate(queueSize,64);
     xTaskCreate(
                    taskOLED,          /* Task function. */
                    "taskOLED",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL); 

}

void loop() {
  delay(5000);
}
