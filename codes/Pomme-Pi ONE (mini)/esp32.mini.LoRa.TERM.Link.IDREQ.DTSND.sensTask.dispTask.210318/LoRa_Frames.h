// this file describes the types of the LoRa frames used to communicate between the LoRa terminals
// and the gateways : ThingSpeak gateway and MQTT gateway
// the identifiers: source-sid and destination-did are derived from the chip identifiers.
// Only 4 lower bytes are taken into account (uint32_t)
// The control frame contain 32 bytes, the data frames are built with 64 bytes
// the 2 bytes of control field con[2] are used to identify the tape of the frame: the control frames and the data frames
// The first con[0] bytes indicates the type of the frame and the type of the gateway (service) to be used.
// The first hexa value of this byte indicates the type of the gateways/service: 

//   1 - ThingSpeak(TS) send, 2 -ThingSpeak(TS) receive
//   3 - MQTT publish, 4 - MQTT subscribe, 
//   5 - TS send and receive, 6 - MQTT publish and subscribe
//   7 - all services

// The second hexa value indicates the type of the packet:

//   01 - IDREQ : ID request for any service and ID acknowledge 
//   11 - IDREQ, 12 - IDACK  : ID request and ID acknowledge - TS send
//   21 - IDREQ, 22 - IDACK  : ID request and ID acknowledge - TS receive
//   31 - IDREQ, 32 - IDACK  : ID request and ID acknowledge - MQTT publish
//   41 - IDREQ, 42 - IDACK  : ID request and ID acknowledge - MQTT subscribe
//   51 - IDREQ, 52 - IDACK  : ID request and ID acknowledge - TS send and receive
//   61 - IDREQ, 62 - IDACK  : ID request and ID acknowledge - MQTT publish and subscribe
//   71 - IDREQ, 72 - IDACK  : ID request and ID acknowledge - all services
//
//   13 - DTSND, 14 - DTACK  : DATA send and DATA acknowledge - TS send
//   23 - DTREQ, 24 - DTRCV  : DATA request and DATA receive - TS receive
//   33 - DTPUB, 34 - DTPUB ACK  : DATA publish and DATA publish acknowledge - MQTT publish
//   43 - DTSUB, 44 - DTSUB RCV  : DATA subscribe and DATA subscribe receive - MQTT subscribe

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

// Example of usage - declaration
// conframe_t scf,rcf,sccf,rccf;  send and recive control packets without and with crypting
// TSframe_t sdf,rdf,scdf,rcdf;  send and recive TS data packets without and with crypting
// MQTTframe_t sdf,rdf,scdf,rcdf;  send and recive MQTT data packets without and with crypting
