
#include <esp_now.h>
#include <WiFi.h>
#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the u8x8 used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16); // data, clock, reset


void InitESPNow() {
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  //If there was an error
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}

//Callback function that tells us when data from Master is received
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  //Copies the sender Mac Address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //Prints it on Serial Monitor
  Serial.print("Received from: "); 
  Serial.println(macStr);
  Serial.println("");
  //sprintf(dbuff,"Sequence",(int)sdf.pack.sequence);
  u8x8.drawString(0,1,"Received");
}

void loop() {
}





void setup() {
  Serial.begin(9600);
  u8x8.begin();  // initialize u8x8
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0,0,"ESP NOW");

  //Puts ESP in STATION MODE
  WiFi.mode(WIFI_STA);
  Serial.print("Mac Address in Station: "); 
  Serial.println(WiFi.macAddress());
  InitESPNow();
  esp_now_register_recv_cb(OnDataRecv);

}

  
  
