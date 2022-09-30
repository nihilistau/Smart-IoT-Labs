#define SS       5 // 26     // D0 - to NSS
#define RST     15  //16     // D4  - RST
#define DI0     26      // D8 - INTR
#define SCK     18      // D5 - CLK
#define MISO    19      // D6 - MISO
#define MOSI    23      // D7 - MOSI
#define BAND    868E6   // set frequency
#define SF      7       // set spreading factor
#define SBW     125E3   // set signal bandwidth
#define SW      0xF3    // set Sync Word
#define BR      8       // set bit rate (4/5,5/8)


void set_LoRa()
{
char buff[128];

  SPI.begin(SCK, MISO, MOSI, SS);  // SCK, MISO, MOSI, SS
  LoRa.setPins(SS, RST, DI0);
  Serial.begin(9600);
  delay(1000);
  Serial.println();
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  sprintf(buff,"BAND=%f,SF=%d,SBW=%f,SW=%X,BR=%d\n",BAND,SF,SBW,SW,BR);
  Serial.println(buff);
  LoRa.setSpreadingFactor(SF);
  LoRa.setSignalBandwidth(SBW); 
  LoRa.setSyncWord(SW);           
  }
  
void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}


