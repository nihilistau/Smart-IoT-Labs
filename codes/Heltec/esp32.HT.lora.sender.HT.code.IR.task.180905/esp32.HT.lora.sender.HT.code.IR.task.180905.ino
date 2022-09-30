
#include <SPI.h>
#include <LoRa.h>
#include <U8x8lib.h>
#include <Wire.h>
#include "SparkFunHTU21D.h"
#include <IRremote.h>

//Create an instance of the object
HTU21D TempHumi;
 
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define freq    870E6  //you can set band here directly,e.g. 868E6,915E6
int  sf=8;
int sb=125E3;       // set the signal bandwidth; 125KHz,[250KHz,500Kz]



int RECV_PIN = 16;

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

IRrecv irrecv(RECV_PIN);

decode_results results;

QueueHandle_t cqueue;



int counter = 0;

union tspack
  {
    uint8_t frame[128];
    struct packet
      {
        uint8_t head[4];
        uint16_t num;
        uint16_t tout;
        float sensor[4];
      } pack;
  } sdf,sbf,rdf,rbf;  // data frame and beacon frame


float humi,temp;
int irr=0;  // code from IR receiver

void dispData()
{
char dbuf[32];  
  u8x8.clear();
  memset(dbuf,0x00,32);
  sprintf(dbuf,"Packet:%4.4d",counter);
  u8x8.drawString(0, 1,dbuf);
  memset(dbuf,0x00,32);
  sprintf(dbuf,"Temp:%2.2f",temp);
  u8x8.drawString(0, 2,dbuf);
  memset(dbuf,0x00,32);
  sprintf(dbuf,"Humi:%2.2f",humi);
  u8x8.drawString(0, 3,dbuf);
}

void dispPara()
{
char dbuf[32];  
  u8x8.clear();
  u8x8.drawString(0, 1, "LoRa Sender");
  sprintf(dbuf,"Freq:%3.2f MHz",(float)freq/1000000.0);
  u8x8.drawString(0, 2, dbuf);
  LoRa.setSpreadingFactor(sf);
  sprintf(dbuf,"SF:%d",sf);
  u8x8.drawString(0, 3, dbuf);
  LoRa.setSignalBandwidth(sb);
  sprintf(dbuf,"SB:%3.2f KHz",(float)sb/1000.0);
  u8x8.drawString(0, 4, dbuf);
  delay(6000);
}



void sensors()
{
  delay(1000);
  humi = TempHumi.readHumidity();
  temp = TempHumi.readTemperature();
}

void LoRaSendTask( void * pvParameters )
{
while(1)
  {  
  sensors();
  LoRa.beginPacket();
  sdf.pack.sensor[0]=(float) counter;;
  sdf.pack.sensor[1]=(float)irr;  // data from IR receiver
  sdf.pack.sensor[2]=temp;
  sdf.pack.sensor[3]=humi;
  LoRa.write(sdf.frame,24);
  LoRa.endPacket();
  dispData();
  Serial.print("Packet:");Serial.println(counter);
  Serial.print("Temp:");Serial.println(temp);
  Serial.print("Humi:");Serial.println(humi);
  
  counter++;
  delay(20000);                       // wait for 6 seconds
  }
}

void IRReadTask( void * pvParameters )
{
  char cbuff[32];
  int i=0;
while(1)
  { 
     if (irrecv.decode(&results)) {
    if(results.value==16738455) { Serial.print("1");cbuff[i++]='1';} 
    if(results.value==16750695) { Serial.print("2");cbuff[i++]='2';}
    if(results.value==16756815) { Serial.print("3");cbuff[i++]='3';}
    if(results.value==16724175) { Serial.print("4");cbuff[i++]='4';}
    if(results.value==16718055) { Serial.print("5");cbuff[i++]='5';}
    if(results.value==16743045) { Serial.print("6");cbuff[i++]='6';}
    if(results.value==16716015) { Serial.print("7");cbuff[i++]='7';}
    if(results.value==16726215) { Serial.print("8");cbuff[i++]='8';}
    if(results.value==16734885) { Serial.print("9");cbuff[i++]='9';}
    if(results.value==16728765) { Serial.print("*");cbuff[i++]='*';}
    if(results.value==16730805) { Serial.print("0");cbuff[i++]='0';}
    if(results.value==16732845) { Serial.print("#");cbuff[i++]='#';}
    if(results.value==16720605) { Serial.print("L");cbuff[i++]='L';}

    if(results.value==16761405) { Serial.print("R");cbuff[i++]='R';}
    if(results.value==16736925) { Serial.print("U");cbuff[i++]='U';}
    if(results.value==16754775) { Serial.print("D");cbuff[i++]='D';}
    if(results.value==16712445) { 
            xQueueSend(cqueue, cbuff, portMAX_DELAY);
            memset(cbuff,0x00,32);i=0;
      }// OK
    irrecv.resume(); // Receive the next value
  }
  delay(100);
  }
}

void setup() {

  Serial.begin(9600);
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");
  Serial.println("LoRa Sender");
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");

  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    u8x8.drawString(0, 1, "Starting LoRa failed!");
    while (1);
  }

  cqueue = xQueueCreate(4, 32); // queue for parameters

  Serial.println("LoRa Sender");
  dispPara();
  Wire.begin(); 
  TempHumi.begin();
  Serial.println("HTU21D Example!");

  xTaskCreate(
                    LoRaSendTask,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL     );  /* Task handle */
 
  Serial.println("LoRaSendTask created...");
  
  xTaskCreate(
                    IRReadTask,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    0,          /* Priority of the task */
                    NULL     );  /* Task handle */
 
  Serial.println("IRReadTask created...");




}


char inbuff[32];
void loop() {
      Serial.println("waiting for item");
      xQueueReceive(cqueue, inbuff, portMAX_DELAY);
      Serial.println(inbuff);
      irr=atoi(inbuff);  
      delay(3000);
}

