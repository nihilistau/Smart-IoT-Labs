#include "LoRaWan_APP.h"
#include "Arduino.h"
#define timetillsleep 1000  // for 1000 3 packets are sent
#define timetillwakeup 15000
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower=1;
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
static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );

typedef enum {LOWPOWER,ReadVoltage,TX} States_t;
States_t state;

bool sleepMode = false;
int16_t rssi,rxSize;
uint16_t voltage;

void onSleep()
{
  Serial.printf("Going into lowpower mode, %d ms later wake up.\r\n",timetillwakeup);
  lowpower=1;       turnOffRGB(); //Radio.Sleep();
  //timetillwakeup ms later wake up;
  TimerSetValue( &wakeUp, timetillwakeup );
  TimerStart( &wakeUp );
}
void onWakeUp()
{
  Serial.printf("Woke up, %d ms later into lowpower mode.\r\n",timetillsleep);
  lowpower=0;
  //timetillsleep ms later into lowpower mode;
  TimerSetValue( &sleep, timetillsleep );
  TimerStart( &sleep );
}

void setup() {
    Serial.begin(9600);
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
    state=ReadVoltage;

    Radio.Sleep( );  // LoRa modem sleep mode
    turnOffRGB();
    TimerInit( &sleep, onSleep );
    TimerInit( &wakeUp, onWakeUp );
    onSleep();
}

void loop()
{
if(lowpower)
  {
    lowPowerHandler();  
  }
else  
  {  
  switch(state)
  {
    case TX:
    {
      sprintf(txPacket,"%s","ADC_battery (mV): ");
      sprintf(txPacket+strlen(txPacket),"%d",voltage);
      if(voltage<(uint16_t)3680)turnOnRGB(COLOR_SEND,0);
      else turnOnRGB(COLOR_RECEIVED,0);
      Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txPacket, strlen(txPacket));
      Radio.Send( (uint8_t *)txPacket, strlen(txPacket) );
      state=LOWPOWER;delay(100);
      break;
    }
    case LOWPOWER:
    {
      Serial.println("going to low power mode");
      delay(100);
      turnOffRGB();
      delay(100);  //LowPower time
      state = ReadVoltage; 
      break;
    }
    case ReadVoltage:
    {
      Serial.println("reading battery voltage");
      pinMode(VBAT_ADC_CTL,OUTPUT);
      digitalWrite(VBAT_ADC_CTL,LOW);
      voltage=analogRead(ADC); //*2;
      pinMode(VBAT_ADC_CTL, INPUT);
      state = TX;
      break;
    }
     default:
          break;
  }
  Radio.IrqProcess();
 }
}

void OnTxDone( void )
{
  Serial.println("TX done!");
  turnOffRGB();Radio.Sleep( );
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    state=ReadVoltage;
    Serial.println(state);
}
