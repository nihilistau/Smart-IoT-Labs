#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <U8x8lib.h>

#define BUILTIN_LED 25

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) {
  memcpy_P(buf, APPEUI, 8);
}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getDevEui (u1_t* buf) {
  memcpy_P(buf, DEVEUI, 8);
}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.
static const u1_t PROGMEM APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getDevKey (u1_t* buf) {
  memcpy_P(buf, APPKEY, 16);
}

static uint8_t mydata[] = "Hi";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 30;

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 18,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 14,
  .dio = {26, 33, 32},
};

void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  u8x8.setCursor(0, 5);
  u8x8.printf("TIME %lu", os_getTime());
  Serial.print(": ");
  switch (ev) {
case EV_SCAN_TIMEOUT:
  Serial.println(F("EV_SCAN_TIMEOUT"));
  u8x8.drawString(0, 7, "EV_SCAN_TIMEOUT");
  break;
case EV_BEACON_FOUND:
  Serial.println(F("EV_BEACON_FOUND"));
  u8x8.drawString(0, 7, "EV_BEACON_FOUND");
  break;
case EV_BEACON_MISSED:
  Serial.println(F("EV_BEACON_MISSED"));
  u8x8.drawString(0, 7, "EV_BEACON_MISSED");
  break;
case EV_BEACON_TRACKED:
  Serial.println(F("EV_BEACON_TRACKED"));
  u8x8.drawString(0, 7, "EV_BEACON_TRACKED");
  break;
case EV_JOINING:
  Serial.println(F("EV_JOINING"));
  u8x8.drawString(0, 7, "EV_JOINING");
  break;
case EV_JOINED:
  Serial.println(F("EV_JOINED"));
  u8x8.drawString(0, 7, "EV_JOINED ");

  // Disable link check validation (automatically enabled
  // during join, but not supported by TTN at this time).
  LMIC_setLinkCheckMode(0);
  break;
case EV_RFU1:
  Serial.println(F("EV_RFU1"));
  u8x8.drawString(0, 7, "EV_RFUI");
  break;
case EV_JOIN_FAILED:
  Serial.println(F("EV_JOIN_FAILED"));
  u8x8.drawString(0, 7, "EV_JOIN_FAILED");
  break;
case EV_REJOIN_FAILED:
  Serial.println(F("EV_REJOIN_FAILED"));
  u8x8.drawString(0, 7, "EV_REJOIN_FAILED");
  //break;
  break;
case EV_TXCOMPLETE:
  Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
  u8x8.drawString(0, 7, "EV_TXCOMPLETE");
  digitalWrite(BUILTIN_LED, LOW);
  if (LMIC.txrxFlags & TXRX_ACK) {
    Serial.println(F("Received ack"));
    u8x8.drawString(0, 7, "Received ACK");
  }
  if (LMIC.dataLen) {
    Serial.println(F("Received "));
    u8x8.drawString(0, 6, "RX ");
    Serial.println(LMIC.dataLen);
    u8x8.setCursor(4, 6);
    u8x8.printf("%i bytes", LMIC.dataLen);
    Serial.println(F(" bytes of payload"));
    u8x8.setCursor(0, 7);
    u8x8.printf("RSSI %d SNR %.1d", LMIC.rssi, LMIC.snr);
  }
  // Schedule next transmission
  os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
  break;
case EV_LOST_TSYNC:
  Serial.println(F("EV_LOST_TSYNC"));
  u8x8.drawString(0, 7, "EV_LOST_TSYNC");
  break;
case EV_RESET:
  Serial.println(F("EV_RESET"));
  u8x8.drawString(0, 7, "EV_RESET");
  break;
case EV_RXCOMPLETE:
  // data received in ping slot
  Serial.println(F("EV_RXCOMPLETE"));
  u8x8.drawString(0, 7, "EV_RXCOMPLETE");
  break;
case EV_LINK_DEAD:
  Serial.println(F("EV_LINK_DEAD"));
  u8x8.drawString(0, 7, "EV_LINK_DEAD");
  break;
case EV_LINK_ALIVE:
  Serial.println(F("EV_LINK_ALIVE"));
  u8x8.drawString(0, 7, "EV_LINK_ALIVE");
  break;
default:
  Serial.println(F("Unknown event"));
  u8x8.setCursor(0, 7);
  u8x8.printf("UNKNOWN EVENT %d", ev);
  break;
  }
}

void do_send(osjob_t* j) {
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
Serial.println(F("OP_TXRXPEND, not sending"));
u8x8.drawString(0, 7, "OP_TXRXPEND, not sent");
  } else {
// Prepare upstream data transmission at the next possible time.
LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 0);
Serial.println(F("Packet queued"));
u8x8.drawString(0, 7, "PACKET QUEUED");
digitalWrite(BUILTIN_LED, HIGH);
  }
  // Next TX is scheduled after TX_COMPLETE event.
}


void setup() {

  Serial.begin(9600);

  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(0, 1, "LoRaWAN LMiC");

  SPI.begin(5, 19, 27);

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  // Start job (sending automatically starts OTAA too)
  do_send(&sendjob);

  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);
}

void loop() {
  os_runloop_once();
}

