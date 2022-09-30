// this file describes the types of the LoRa frames used to communicate between the LoRa terminals
// and the gateways : ThingSpeak gateway and MQTT gateway
// the identifiers: source-sid and destination-did are derived from the chip identifiers.
// Only 4 lower bytes are taken into account (uint32_t)
// The control frame contain 32 bytes, the data frames are built with 64 bytes
// the 2 bytes of control field con[2] are used to identify the tape of the frame: the control frames and the data frames
// The first con[0] bytes indicates the type of the frame and the type of the gateway (service) to be used.
// The first hexa value of this byte indicates the type of the gateways/service: 

//   1 - ThingSpeak(TS) send, 2 -ThingSpeak(TS) receive
//   3 - MQTT - MQP publish, 4 - MQTT - MQS subscribe, 
//   5 - TS send and receive, 6 - MQTT publish and subscribe
//   7 - all services

// The second hexa value indicates the type of the packet:

//   0x01 - IDREQ : ID request for any service and ID acknowledge 
//   0x11 - IDREQ, 0x12 - IDACK  : ID request and ID acknowledge - TS send - TSS
//   0x21 - IDREQ, 0x22 - IDACK  : ID request and ID acknowledge - TS receive -TSR
//   0x31 - IDREQ, 0x32 - IDACK  : ID request and ID acknowledge - MQTT publish - MQP
//   0x41 - IDREQ, 0x42 - IDACK  : ID request and ID acknowledge - MQTT subscribe - MQS
//   0x51 - IDREQ, 0x52 - IDACK  : ID request and ID acknowledge - TS send and receive - TSSR
//   0x61 - IDREQ, 0x62 - IDACK  : ID request and ID acknowledge - MQTT publish and subscribe - MQPS
//   0x71 - IDREQ, 0x72 - IDACK  : ID request and ID acknowledge - all services - TSMQALL
//
//   0x13 - DTSND, 0x14 - DTACK  : DATA send and DATA acknowledge - TS send  - TSS
//   0x23 - DTREQ, 0x24 - DTRCV  : DATA request and DATA receive - TS receive - TSR
//   0x33 - DTPUB, 0x34 - DTPUB ACK  : DATA publish and DATA publish acknowledge - MQTT publish - MQP
//   0x43 - DTSUB, 0x44 - DTSUB RCV  : DATA subscribe and DATA subscribe receive - MQTT subscribe - MQS

#define IDREQ_ALL 0x01
#define IDREQ_TSS 0x11
#define IDREQ_TSR 0x21
#define IDREQ_MQP 0x31
#define IDREQ_MQS 0x41

#define IDACK_ALL 0x02
#define IDACK_TSS 0x12
#define IDACK_TSR 0x22
#define IDACK_MQP 0x32
#define IDACK_MQS 0x42

#define DTSND 0x13  // TSS
#define DTREQ 0x23  // TSR
#define DTPUB 0x33  // MQP
#define IDSUB 0x43  // MQS

#define DTACK 0x14  // TSS
#define DTRCV 0x24  // TSR
#define DTPUBACK 0x34  // MQP
#define DTSUBRCV 0x44  // MQS


// The second byte of the control field con[1] is used to carry the data fields mask (ThingSpeak)
// Each bit of this field corresponds to one data value of ThingSpeak channel; 
// With this mask the fields may be specified when sending or requesting data

// The password field pass[16] is used by the control frames IDREQ and IDACK to protect the access
// to the gateway node; only a frame with correct password (16 bytes) is to be answered by IDACK frame
// The tout value coded on 16 bits may be used to synchronize the terminals operating in deep sleep mode.
// The gateway none sends the calculated delay according to its scheduler (agenda) to the terminal node.

// The data frames are different for ThingSpeak serviecs and for MQTT services.
// The data frame relayed by the gateway node to ThingSpeak server contains the 

// The following AES encryp and decrypt function are defined to protect the packets with a symetric crypt keyword

#include "mbedtls/aes.h"

void encrypt(unsigned char *plainText,char *key,unsigned char *outputBuffer, int nblocks)
{
  mbedtls_aes_context aes;
  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_enc(&aes,(const unsigned char*)key,strlen(key)*8);
  for(int i=0;i<nblocks;i++)
    {
    mbedtls_aes_crypt_ecb(&aes,MBEDTLS_AES_ENCRYPT,
                        (const unsigned char*)(plainText+i*16), outputBuffer+i*16);
    }                 
  mbedtls_aes_free(&aes);
}

void decrypt(unsigned char *chipherText,char *key,unsigned char *outputBuffer, int nblocks)
{
  mbedtls_aes_context aes;
  mbedtls_aes_init( &aes );
  mbedtls_aes_setkey_dec( &aes, (const unsigned char*) key, strlen(key) * 8 );
    for(int i=0;i<nblocks;i++)
    {
    mbedtls_aes_crypt_ecb(&aes,MBEDTLS_AES_DECRYPT,
                       (const unsigned char*)(chipherText+i*16), outputBuffer+i*16);
    }                   
  mbedtls_aes_free(&aes );
}

typedef union 
{
  uint8_t frame[32];
  struct
  {
    uint32_t did;       // destination identifier chipID (4 lower bytes)
    uint32_t sid;       // source identifier chipID (4 lower bytes)
    uint8_t  con[2];    // control field: con[0]
    char     pass[16];  // password - 16 characters
    uint16_t tout;      // timeout
    uint8_t  pad[4];    // future use
  } pack;               // control packet
} conframe_t;              // send control frame , receive control frame

typedef union 
{
  uint8_t frame[64];    // TS frame to send/receive data
  struct
  {
    uint32_t did;        // destination identifier chipID (4 lower bytes)
    uint32_t sid;        // source identifier chipID (4 lower bytes)
    uint8_t  con[2];     // control field: lower byte is used as mask
    int     channel;     // TS channel number
    char    keyword[16];   // write (or read) keyword
    float   sens[8];     // max 8 values – fields
    uint16_t  tout;      // optional timeout
  } pack;                // data packet
} TSframe_t; 

typedef union 
{
  uint8_t frame[64];    // MQTT frame to publish on the given topic
  struct
  {
    uint32_t did;        // destination identifier chipID (4 lower bytes)
    uint32_t sid;        // source identifier chipID (4 lower bytes)
    uint8_t  con[2];     // control field
    char     topic[24];  // topic name - e.g. /esp32/Term1/Sens1
    char     mess[24];   // message value
    uint16_t tout;      // optional timeout for publish frame
    uint8_t  pad[4];     // future use
  } pack;                // data packet
} MQTTframe_t;

void send_IDREQ(uint32_t termID,uint8_t code, char *pass, char *crypt)  // code - MODE: TSS(1),TSR(2),MQP(3),MQS(4)
{
conframe_t scf,sccf;uint8_t serv;
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  scf.pack.did=(uint32_t)0; scf.pack.sid=(uint32_t)termID;
  serv=code*16+1;
  scf.pack.con[0]=serv;scf.pack.con[1]=0x00;   // 0x11 - IDREQ packet (1)
  if(pass!=NULL)
     strncpy(scf.pack.pass,"passwordpassword",16);
  else 
    memcpy(scf.pack.pass,0x00,16);   
  if(crypt!=NULL)
    {
    encrypt(scf.frame,crypt,sccf.frame,2);LoRa.write(sccf.frame,32);
    }
  else LoRa.write(scf.frame,32);              
  LoRa.endPacket(true);                  
}

void send_DTSND(uint32_t gwID,uint32_t termID,uint8_t mask,int chan,char *wkey,float *stab, char *crypt)
{
TSframe_t sdf,sdcf;  
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)gwID;    // receiver (GW) ID
  sdf.pack.sid=(uint32_t)termID;
  sdf.pack.con[0]=0x13; sdf.pack.con[1]=mask;       // DTSND
  sdf.pack.channel= chan;
  strncpy(sdf.pack.keyword,wkey,16);
  for(int i=0;i<8;i++) sdf.pack.sens[i]=stab[i];
  sdf.pack.tout=0;
  if(crypt!=NULL)
    {
    encrypt(sdf.frame,crypt,sdcf.frame,4);LoRa.write(sdcf.frame,64);
    }
  else LoRa.write(sdf.frame,64);
  LoRa.endPacket(true);                 // finish packet and send it
}

void send_DTREQ(uint32_t gwID,uint32_t termID,uint8_t mask,int chan,char *rkey, char *crypt)
{
TSframe_t sdf,sdcf;  
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)gwID;    // receiver (GW) ID
  sdf.pack.sid=(uint32_t)termID;
  sdf.pack.con[0]=0x23; sdf.pack.con[1]=mask;       // DTREQ
  sdf.pack.channel= chan;
  strncpy(sdf.pack.keyword,rkey,16);
  for(int i=0;i<8;i++) sdf.pack.sens[i]=0.0;
  sdf.pack.tout=0;
  if(crypt!=NULL)
    {
    encrypt(sdf.frame,crypt,sdcf.frame,4);LoRa.write(sdcf.frame,64);
    }
  else LoRa.write(sdf.frame,64);
  LoRa.endPacket(true);                 // finish packet and send it
}

void send_DTPUB(uint32_t gwID,uint32_t termID,char *topic, char *mess, char *crypt)  // send DTPUB packet
{
MQTTframe_t sdf,sdcf; 
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)gwID;     
  sdf.pack.sid=(uint32_t)termID;
  sdf.pack.con[0]=0x33; sdf.pack.con[1]=0x00;   // DTPUB packet     
  strcpy(sdf.pack.topic,topic);
  strcpy(sdf.pack.mess,mess);
  sdf.pack.tout=0;
  if(crypt!=NULL)
    {
    encrypt(sdf.frame,crypt,sdcf.frame,4);LoRa.write(sdcf.frame,64);
    }
  else LoRa.write(sdf.frame,64);
  LoRa.endPacket(true);                  
}

void send_DTPUBACK(uint32_t gwID,uint32_t termID,char *topic,char *mess, char *crypt)  // send DTSUB RCVed data
{
MQTTframe_t sdf,sdcf; 
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)termID;     
  sdf.pack.sid=(uint32_t)gwID;
  sdf.pack.con[0]=0x34; sdf.pack.con[1]=0x00;        // MQTT SUB - receive
  strncpy(sdf.pack.topic,topic,24);
  strncpy(sdf.pack.mess,mess,24);
  sdf.pack.tout=0;
  if(crypt!=NULL)
    {
    encrypt(sdf.frame,crypt,sdcf.frame,4);LoRa.write(sdcf.frame,64);
    }
  else LoRa.write(sdf.frame,64);
  LoRa.endPacket(true);                  
}


void send_DTSUB(uint32_t gwID,uint32_t termID,char *topic, char *crypt)  // send DTSUB frame - subscribe topic
{
MQTTframe_t sdf,sdcf; 
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)gwID;    // receiver (GW) ID
  sdf.pack.sid=(uint32_t)termID;
  sdf.pack.con[0]=0x43; sdf.pack.con[1]=0x00;        
  strncpy(sdf.pack.topic,topic,24);
  memset(sdf.pack.mess,0x00,24);
  sdf.pack.tout=0;
  if(crypt!=NULL)
    {
    encrypt(sdf.frame,crypt,sdcf.frame,4);LoRa.write(sdcf.frame,64);
    }
  else LoRa.write(sdf.frame,64);
  LoRa.endPacket(true);                 // finish packet and send it
}

void send_IDACK(uint32_t termID,uint32_t gwID,uint8_t code,uint16_t tout, char *crypt)  // code - MODE: TSS(1),TSR(2),MQP(3),MQS(4)       
{
conframe_t scf,sccf; uint8_t serv;
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  scf.pack.did=(uint32_t)termID;  // ack for IDREQ frame
  scf.pack.sid=(uint32_t)gwID;  // send ACK frame with the received nodeID
  serv=16*code+2;
  scf.pack.con[0]=serv;scf.pack.con[1]=0x00;  // serv=0x12,0x22,0x32,0x42 DTACK for this service
  strncpy(scf.pack.pass,"passwordpassword",16);
  scf.pack.tout=tout;                 // timeout for the next packet
  if(crypt!=NULL)
    {
    encrypt(scf.frame,crypt,sccf.frame,2);LoRa.write(sccf.frame,32);
    }
  else LoRa.write(scf.frame,32);              
  LoRa.endPacket(true);                  
}

void send_DTACK(uint32_t termID,uint32_t gwID,uint8_t mask,int chan,char *wkey,uint16_t tout, char *crypt)  // TS -send (1)
{
TSframe_t sdf,sdcf; 
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                    
  sdf.pack.did=(uint32_t)termID;   
  sdf.pack.sid=(uint32_t)gwID;   
  sdf.pack.con[0]=0x14;sdf.pack.con[1]=mask;   
               // calculate timeout for the next data frame
  sdf.pack.channel= chan;
  if (wkey!=NULL)
    strncpy(sdf.pack.keyword,wkey,16);
  for(int i=0;i<8;i++) sdf.pack.sens[i]=0.0;
  sdf.pack.tout=tout;
  if(crypt!=NULL)
    {
    encrypt(sdf.frame,crypt,sdcf.frame,4);LoRa.write(sdcf.frame,64);
    }
  else LoRa.write(sdf.frame,64);              
  LoRa.endPacket(true);                  
}

void send_DTRCV(uint32_t termID,uint32_t gwID,uint8_t mask,int chan,char *rkey,float stab[],uint16_t tout, char *crypt)  // TS -receive (2)
{
TSframe_t sdf,sdcf; 
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  sdf.pack.did=(uint32_t)termID;  // ack for DTSND frame 
  sdf.pack.sid=(uint32_t)gwID;  // send ACK frame with the received nodeID
  sdf.pack.con[0]=0x24;sdf.pack.con[1]=mask;  // con[0] depends on the type af ACK frame: IDREQ,DTACK,..
               // calculate timeout for the next data frame
  sdf.pack.channel= chan;
  strncpy(sdf.pack.keyword,rkey,16);
  for(int i=0;i<8;i++) sdf.pack.sens[i]=stab[i];
  sdf.pack.tout=tout;
  if(crypt!=NULL)
    {
    encrypt(sdf.frame,crypt,sdcf.frame,4);LoRa.write(sdcf.frame,64);
    }
  else LoRa.write(sdf.frame,64);               
  LoRa.endPacket(true);                  
}

void send_DTSUBRCV(uint32_t gwID,uint32_t termID,char *topic,char *mess, char *crypt)  // send DTSUB RCVed data
{
MQTTframe_t sdf,sdcf; 
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();
  sdf.pack.did=(uint32_t)termID;     
  sdf.pack.sid=(uint32_t)gwID;
  sdf.pack.con[0]=0x44; sdf.pack.con[1]=0x00;        // MQTT SUB - receive
  strncpy(sdf.pack.topic,topic,24);
  strncpy(sdf.pack.mess,mess,24);
  sdf.pack.tout=0;
  if(crypt!=NULL)
    {
    encrypt(sdf.frame,crypt,sdcf.frame,4);LoRa.write(sdcf.frame,64);
    }
  else LoRa.write(sdf.frame,64);
  LoRa.endPacket(true);                  
}






