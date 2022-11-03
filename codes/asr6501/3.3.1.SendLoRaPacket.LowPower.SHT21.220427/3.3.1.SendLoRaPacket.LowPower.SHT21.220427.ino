
#include "Arduino.h"

#define LoraWan_RGB 5
#include "LoRaWan_APP.h"
#include <Wire.h>
#include "SHT21.h"
SHT21 SHT21;

#define RF_FREQUENCY                                868500000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm - 25mW, 17-50mW, 20 - 100mW
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]                                                       //  3: Reserved]
#define LORA_SPREADING_FACTOR                       9         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txPacket[BUFFER_SIZE];
static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );

uint32_t timetillsleep=5000;
uint32_t timetillwakeup=12000;
static TimerEvent_t sleepGo;
static TimerEvent_t wakeUp;
uint8_t lowpower=1, highpower=0;
int16_t txNumber;

bool sleepMode = false;
int16_t rssi,rxSize;
uint16_t voltage;

float t,h;
int dt,dh;
char buff[32];

union 
{
uint8_t frame[16];
float sensor[4];
} sdp; // send data packet


void getSHT21()
{
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    delay(200);
    Wire.begin(29,28);
    SHT21.begin();
    sdp.sensor[0]=SHT21.getTemperature();
    sdp.sensor[1]=SHT21.getHumidity();
    delay(100);
    Wire.end();
    Serial.println(sdp.sensor[0]);Serial.println(sdp.sensor[1]);
    digitalWrite(Vext, HIGH);
}



void onSleep()
{
  Serial.printf("Going into lowpower mode, %d ms later wake up.\r\n",timetillwakeup);
  lowpower=1;highpower=0;
  //timetillwakeup ms later wake up;
  TimerSetValue( &wakeUp, timetillwakeup );
  TimerStart( &wakeUp );
}

void onWakeUp()
{
  Serial.printf("Woke up, %d ms later into lowpower mode.\r\n",timetillsleep);
  lowpower=0;highpower=1; //turnOnRGB(0,0);
  //timetillsleep ms later into lowpower mode;
  TimerSetValue( &sleepGo, timetillsleep );
  TimerStart( &sleepGo );
}

void OnTxDone( void )
{
  Serial.println("TX done!");
  Radio.Sleep( );
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("TX Timeout......");
}


void setup() {
    Serial.begin(9600);
    pinMode(Vext,OUTPUT);
    digitalWrite(Vext,LOW); //SET POWER
    voltage = 0;
    rssi=0;
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

  //state=ReadVoltage;
  Radio.Sleep( );
  TimerInit( &sleepGo, onSleep );  // activate on event
  TimerInit( &wakeUp, onWakeUp );
  onSleep();
}

long debut,del=2000;  // the receiver must respond in this waiting period


void loop() {
  if(lowpower){
    lowPowerHandler();
  }
  if(highpower)
     { 
      Serial.println("in highpower");
      getSHT21();
      pinMode(VBAT_ADC_CTL,OUTPUT);
      digitalWrite(VBAT_ADC_CTL,LOW);
      voltage=analogRead(ADC)+400;  
      pinMode(VBAT_ADC_CTL, INPUT);
      sprintf(txPacket,"V:%d,T:%d,H:%d",voltage,(int)sdp.sensor[0],(int)sdp.sensor[1]);
      Serial.printf("sending packet [%s], length: %d\n",txPacket,strlen(txPacket));
      delay(100);
      Radio.Send( (uint8_t *)txPacket, strlen(txPacket) );
      highpower=0;lowpower=1; 

    }
 Radio.IrqProcess( );
}
