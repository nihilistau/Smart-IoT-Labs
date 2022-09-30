HardwareSerial SIM900(2);
/* Create object named SIM900 of the class SoftwareSerial */
void setup() {
  SIM900.begin(9600);  /* Define baud rate for software serial communication */
  Serial.begin(9600); /* Define baud rate for serial communication */
}


int startgpsr(char *request)
{
char req[128], preq[128]; char buff[128];
//memset(buff,0x00,128);
//memset(req,0x00,128);memset(preq,0x00,128);
//strcpy(req,"GET /update?key=");strcpy(preq,"GET /update?key=");
//strcat(req,request);strcat(preq,request);
strcpy(req,request);strcat(req,"\x1A");
Serial.println(req);

  Serial.println("AT\\r\\n");
  SIM900.println("AT"); /* Check Communication */
  delay(2000);
  Serial.println("AT+CSTT=\"free\",\"\",\"\"\\r\\n");
  SIM900.println("AT+CSTT=\"free\",\"\",\"\"");  /* Start task and set APN */
  delay(5000);
  ShowSerialData();
  delay(2000);
  Serial.println("AT+CIICR\\r\\n");
  SIM900.println("AT+CIICR"); /* Bring up wireless connection with GPRS */
  delay(2000);
  ShowSerialData();
  delay(2000);
  Serial.println("AT+CIFSR\\r\\n");
  SIM900.println("AT+CIFSR"); /* Get local IP address */
  ShowSerialData();  /* Print response on the serial monitor */
  delay(2000);
  Serial.println("AT+QMTCFG=\"recv/mode\",0,0,1\\r\\n"); 
  SIM900.println("AT+QMTCFG=\"recv/mode\",0,0,1"); /* Non-Transparent (normal) mode for TCP/IP application */
  delay(5000);
  ShowSerialData();
  delay(2000);
  Serial.println("AT+QMTOPEN=0,\"broker.emqx.io\",1883\\r\\n");
  SIM900.println("AT+QMTOPEN=0,\"broker.emqx.io\",1883");  /* Single TCP/IP connection mode */
  delay(5000);
  ShowSerialData();
  delay(2000);
  Serial.println("AT+QMTOPEN?\\r\\n");
  SIM900.println("AT+QMTOPEN?"); /* Attach to GPRS Service */
  delay(5000);
  ShowSerialData();
  delay(2000);
  Serial.println("AT+QMTCONN=0,\"mqttTest\"\\r\\n");
  SIM900.println("AT+QMTCONN=0,\"mqttTest\""); /* Network registration status */
  delay(2000);
  ShowSerialData();
  delay(2000);
  Serial.println("AT+QMTPUBEX=0,0,0,0,\"esp32/my_sensors\",5,\"hello\"\\r\\n");
  SIM900.println("AT+QMTPUBEX=0,0,0,0,\"esp32/my_sensors\",5,\"hello\"");  /* Attached to or detached from GPRS service */ 
  delay(5000); 
  ShowSerialData();
  delay(2000);
  ShowSerialData();
  delay(2000);
  return 0;
}

void ShowSerialData()
{
  while(SIM900.available()!=0)  /* If data is available on serial port */
  Serial.write(char (SIM900.read())); /* Print character received on to the serial monitor */
}

int count1=0,count2=0;

void loop() {
  char cbuff[128];
  memset(cbuff,0x00,128);
  startgpsr(cbuff);
}
