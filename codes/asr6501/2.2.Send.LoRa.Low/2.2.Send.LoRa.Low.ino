
#include "Arduino.h"
#include "LoRaWan_APP.h"
#include <Wire.h>
#include "SHT21.h"

SHT21 SHT21;

//#define timetillsleep 5000
//#define timetillwakeup 12000
uint32_t timetillsleep=5000;
uint32_t timetillwakeup=12000;
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower=1, highpower=0;

#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

#define RF_FREQUENCY                                868500000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
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
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );

int16_t txNumber;

int16_t rssi,rxSize;

union 
{
uint8_t frame[16];
float sensor[4];
} sdp; // send data packet


void getSHT21()
{
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    delay(50);
    Wire.begin(29,28);
    SHT21.begin();
    sdp.sensor[0]=SHT21.getTemperature();
    sdp.sensor[1]=SHT21.getHumidity();
    Wire.end();
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
  lowpower=0;highpower=1;
  //timetillsleep ms later into lowpower mode;
  TimerSetValue( &sleep, timetillsleep );
  TimerStart( &sleep );
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  RadioEvents.RxDone = OnRxDone;

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;

  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
  Radio.Sleep( );
  TimerInit( &sleep, onSleep );
  TimerInit( &wakeUp, onWakeUp );
  onSleep();
}

long debut,del=2000;  // the receiver must respond in this waiting period
uint16_t voltage;

void loop() {
  if(lowpower){
    lowPowerHandler();
  }
  if(highpower)
     { 
      char buffp[32];
      turnOnRGB(COLOR_SEND,0);
      getSHT21(); //strcpy(buff,"hello");
      voltage=getBatteryVoltage(); 
      sdp.sensor[2]=(float)((int)voltage);
      Serial.println(voltage);Serial.println(sdp.sensor[2]);
      Radio.Send(sdp.frame, 16);
      debut=millis();delay(300);turnOffRGB();Radio.Rx(0);
      while(millis()<(debut+del))
        {
        Radio.IrqProcess( ); delay(100);
        }
       highpower=0; 
    }

}

uint32_t tout;
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    rssi=rssi;
    rxSize=size;
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';
    tout=atoi(rxpacket);
    turnOnRGB(COLOR_RECEIVED,0);
    timetillwakeup=tout;
    Radio.Sleep( );
    Serial.printf("\r\nreceived packet: %d rssi: %d , length: %d\n",tout,rssi,rxSize);delay(100);
    turnOffRGB();
}

void OnTxDone( void )
{
  Serial.println("TX done!");
  turnOffRGB();
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("TX Timeout......");
}
