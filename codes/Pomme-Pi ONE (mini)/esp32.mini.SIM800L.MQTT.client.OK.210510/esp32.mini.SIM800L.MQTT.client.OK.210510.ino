HardwareSerial SIM800L(2);

#define TINY_GSM_MODEM_SIM800
#define SerialMon Serial
#define SerialAT SIM800L

#define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 9600


// Define how you're planning to connect to the internet
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[] = "free";
const char gprsUser[] = "";
const char gprsPass[] = "";

// MQTT details
const char* broker = "broker.emqx.io";

const char* topicLed = "/esp32/my_sensors/";
const char* topicInit = "/esp32/my_sensors/";
const char* topicLedStatus = "/esp32/my_sensors/";

#include <TinyGsmClient.h>
#include <PubSubClient.h>

// Just in case someone defined the wrong thing..
//#if TINY_GSM_USE_GPRS && not defined TINY_GSM_MODEM_HAS_GPRS
//#undef TINY_GSM_USE_GPRS
//#undef TINY_GSM_USE_WIFI
//#define TINY_GSM_USE_GPRS false
//#define TINY_GSM_USE_WIFI true
//#endif
//#if TINY_GSM_USE_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
//#undef TINY_GSM_USE_GPRS
//#undef TINY_GSM_USE_WIFI
//#define TINY_GSM_USE_GPRS true
//#define TINY_GSM_USE_WIFI false
//#endif
//
//#ifdef DUMP_AT_COMMANDS
//#include <StreamDebugger.h>
//  StreamDebugger debugger(SerialAT, SerialMon);
//  TinyGsm modem(debugger);
//#else
TinyGsm modem(SerialAT);
//#endif
TinyGsmClient client(modem);
PubSubClient mqtt(client);

#define LED_PIN 22
int ledStatus = LOW;

uint32_t lastReconnectAttempt = 0;

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();

  // Only proceed if incoming message's topic matches
  if (String(topic) == topicLed) {
    ledStatus = !ledStatus;
    digitalWrite(LED_PIN, ledStatus);
    //mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
  }
}

boolean mqttConnect() {
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);

  // Connect to MQTT Broker
  boolean status = mqtt.connect("GsmClientTest");

  // Or, if you want to authenticate MQTT:
  //boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");

  if (status == false) {
    SerialMon.println(" fail");
    return false;
  }
  SerialMon.println(" success");
  mqtt.publish(topicInit, "GsmClientTest started");
  mqtt.subscribe(topicLed);
  return mqtt.connected();
}


void setup() {
  // Set console baud rate
  SerialMon.begin(9600);
  delay(10);

  pinMode(LED_PIN, OUTPUT);

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  SerialMon.println("Wait...");

  // Set GSM module baud rate
  //TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  SerialAT.begin(9600);
  delay(6000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();
  // modem.init();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

//#if TINY_GSM_USE_GPRS
//  // Unlock your SIM card with a PIN if needed
//  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
//    modem.simUnlock(GSM_PIN);
//  }
//#endif
//
//#if TINY_GSM_USE_WIFI
//    // Wifi connection parameters must be set before waiting for the network
//  SerialMon.print(F("Setting SSID/password..."));
//  if (!modem.networkConnect(wifiSSID, wifiPass)) {
//    SerialMon.println(" fail");
//    delay(10000);
//    return;
//  }
//  SerialMon.println(" success");
//#endif

//#if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_XBEE
//  // The XBee must run the gprsConnect function BEFORE waiting for network!
//  modem.gprsConnect(apn, gprsUser, gprsPass);
//#endif

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

//#if TINY_GSM_USE_GPRS
  // GPRS connection parameters are usually set after network registration
    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      SerialMon.println(" fail");
      delay(10000);
      return;
    }
    SerialMon.println(" success");

  if (modem.isGprsConnected()) {
    SerialMon.println("GPRS connected");
  }
//#endif

  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
}

void loop() {

  if (!mqtt.connected()) {
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    uint32_t t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) {
        lastReconnectAttempt = 0;
      }
    }
    delay(100);
    return;
  }

  mqtt.loop();
}
