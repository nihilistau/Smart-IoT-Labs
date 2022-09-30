

#include <AT24Cxx.h>
#define i2c_address 0x50

union {
  uint8_t eeprom[20];
  struct {
    long frequency;
    int sf;        // spreading factor
    long sbw;      // signal bandwidth
    int denominator;   // coding rate 4/denominator
    int sw;    // synchronization word
  } lora;
} wpar,rpar;

// start reading from the first byte (address 0) of the EEPROM
int address = 0;
byte value;

uint16_t addr=0; uint8_t v=0;

// Initilaize using AT24CXX(i2c_address, size of eeprom in KB).  
AT24Cxx eep(i2c_address, 32);


void setup() {
Serial.begin(9600);
wpar.lora.frequency =868E6;
wpar.lora.sf=8;
wpar.lora.sbw=125E3;
wpar.lora.denominator=5; 
wpar.lora.sw=0xF3;

for (int i=0;i<20; i++)
{
  eep.write(i,wpar.eeprom[i]);
}

for (int i=0;i<20; i++)
{
rpar.eeprom[i]=eep.read(i);
}

Serial.println(rpar.lora.frequency);Serial.println(rpar.lora.sf);Serial.println(rpar.lora.sbw);
Serial.println(rpar.lora.denominator);Serial.println(rpar.lora.sw);
}

void loop() {
 
  delay(500);
}
