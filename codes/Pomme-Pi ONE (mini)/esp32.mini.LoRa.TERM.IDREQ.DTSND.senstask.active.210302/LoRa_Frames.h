// this file describes the types of the LoRa frames used to communicate between the LoRa terminals
// and the gateways : ThingSpeak gateway and MQTT gateway
// the identifiers: source-sid and destination-did are derived from the chip identifiers.
// Only 4 lower bytes are taken into account (uint32_t)
// The control frame contain 32 bytes, the data frames are built with 64 bytes
// the 2 bytes of control field con[2] are used to identify the tape of the frame: the control frames and the data frames
// The first con[0] bytes indicates the type of the frame and the type of the gateway (service) to be used.
// The first hexa value of this byte indicates the type of the gateways/service: 
//   0 - ThingSpeak send, 1 -ThingSpeak receive
//   2 - MQTT publish, 3 - MQTT subscribe
// The second hexa value indicates the type of the frame:
//   1 - IDREQ, 2 - IDACK  : ID request and ID acknowledge - control frames
//   3 - DTSND, 4 - DTACK  : DATA send (ThingSpeak) and DATA acknowledge - data and control frames
//   5 - DTREQ, 6 - DTRCV  : DATA request and DATA receive - control and data frames
//   7 - DTPUB, 8 - DTSUB, DTRCV : TOPIC/DATA publish, TOPIC subscribe, DATA receive frames (MQTT)
// The second byte of the control field con[1] is used to carry the data fields mask (ThingSpeak)
// Each bit of this field corresponds to one data value of ThingSpeak channel; 
// With this mask the fields may be specified when sending or requesting data

// The password field pass[16] is used by the control frames IDREQ and IDACK to protect the access
// to the gateway node; only a frame with correct password (16 bytes) is to be answered by IDACK frame
// The tout value coded on 16 bits may be used to synchronize the terminals operating in deep sleep mode.
// The gateway none sends the calculated delay according to its scheduler (agenda) to the terminal node.

// The data frames are different for ThingSpeak serviecs and for MQTT services.
// The data frame relayed by the gateway node to ThingSpeak server contains the 



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
