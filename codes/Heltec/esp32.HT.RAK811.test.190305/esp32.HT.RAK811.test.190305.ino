HardwareSerial ss(2);

void setup() {

//configure Serial1, this could also be a 
//software serial. 
ss.begin(9600);
//configure the mail RX0 and TX0 port on arduino
Serial.begin(9600);

}

//string to hold the response of a command in rak811
String response = "";

//the famous arduino loop function. runs continuosly
void loop() {

  //try getting the version of the board firmware
  sendCommand("at+version\r\n");
  //set conn config
  setConnConfig("dev_eui", "your_dev_eui_here");
  setConnConfig("app_eui", "your_app_eui_here");
  setConnConfig("app_key", "your_app_key_here");
  //join the connection
  sendJoinReq();
  //send data too gateway
  sendData(1,2,"000000000000007F0000000000000000");
  delay(3000);

}

/**
 * Function to send a command to the 
 * lora node and wait for a response
 */
void sendCommand(String atComm){
response = "";
ss.print(atComm);
  while(ss.available()){
    char ch = ss.read();
    response += ch;
  }
  Serial.println(response);
}

/**
 * send the rak811 to sleep for time 
 * specified in millis paramteer
 */
void sleep(unsigned long milliseconds){
  sendCommand("at+sleep\r\n");
  delay(milliseconds);
  //send any charcater to wakeup;
  sendCommand("***\r\n");
}

/**
 * reset board after the specified time delay millisenconds
 * <mode> = 0 Reset and restart module
= 1 Reset LoRaWAN or LoraP2P stack and Module will reload LoRa
configuration from EEPROM
 */
void resetChip(int mode, unsigned long delaySec=0){
  delay(delaySec);
  String command = (String)"at+reset=" + mode + (String)"\r\n";
  sendCommand(command);
}

/**
 * Reload the default parameters of LoraWAN or LoraP2P setting
 */
void reload(unsigned long delaySec){
  delay(delaySec);
  sendCommand("at+reload\r\n");
}

/**
 * Function to set module mode
 * <mode> = 0 LoraWAN Mode (default mode)
= 1 LoraP2P Mode
 */
void setMode(int mode){
  String command = (String)"at+mode=" + mode + (String)"\r\n";
  sendCommand(command);  
}

/**
 * Function to send data to a lora gateway;
 * <type> = 0 send unconfirmed packets
= 1 send confirmed packets
<port> = 1-223 port number from 1 to 223
<data>= <hex value> hex value(no space). The Maximum length of <data> 64 bytes
 */
 
void sendData(int type, int port, String data){
  String command = (String)"at+send=" + type + "," + port + "," + data + (String)"\r\n";
  sendCommand(command);
}

/**
 * Function to set the connection config
 * < dev_addr >:<address>
<address>-------------------4 bytes hex number representing the
device address from 00000001 –
FFFFFFFE

<dev_eui>:<eui>
<eui>-------------------------- 8-byte hexadecimal number
representing the device EUI

<app_eui>:<eui>
<eui>----------------------------8-byte hexadecimal number
representing the application EUI

<app_key>:<key>
- 11 -
<key>----------------------------16-byte hexadecimal number
representing the application key

<nwks_key>:<key>
<key>-------------------------16-byte hexadecimal number
representing the network session key

<apps_key>:<key>
<key>------------------------ 16-byte hexadecimal number representing
the application session key

<tx_power>:<dbm>
<dbm>------------------- LoRaWAN Tx Power

<adr>:<status>
<status>----------------------------- string value representing
the state, either “on” or “off”.

<dr>:<data rate>
<data rate>-----------------------decimal number representing the
data rate, from 0 and 4, but within
the limits of the data rate range for
the defined channels.

< public_net >:<status>
<status>------------------- string value representing
the state, either “on” or “off”.

< rx_delay1 >:<delay>
<delay>-------------------decimal number representing
the delay between the transmission
and the first Reception window
in milliseconds, from 0 to 65535.
 */
void setConnConfig(String key, String value){
  sendCommand("at+set_config=" + key + ":" + value + "\r\n");
}

/**
 * Function to join the connection with the specified parameters
 * you should get OK
at+recv=3,0,0 as response.
 */
 void sendJoinReq(){
  sendCommand("at+join\r\n");
 }

