HardwareSerial SIM800L(2);
void setup() {
  SIM800L.begin(9600);  /* Define baud rate for software serial communication */
  Serial.begin(9600); /* Define baud rate for serial communication */
}

float req_HTTP(char *req)
{
  float ret=0.0;
  Serial.print("AT+HTTPINIT\\r\\n");
  SIM800L.println("AT+HTTPINIT"); /* Initialize HTTP service */
  delay(5000); 
  ShowSerialData();
  delay(4000);
  Serial.print("AT+HTTPPARA=\"CID\",1\\r\\n");
  SIM800L.println("AT+HTTPPARA=\"CID\",1");  /* Set parameters for HTTP session */
  delay(4000);
  ShowSerialData();
  delay(4000);
  char cbuff[128]; 
  strcpy(cbuff,"AT+HTTPPARA=\"URL\",");
  strcat(cbuff,req);
  Serial.print("AT+HTTPPARA=\"URL\",\"86.217.14.104:443/channels/174/fields/1/last?key=V7M6I771U8OJ2JSV\"\\r\\n");
  SIM800L.println(cbuff);/* Set parameters for HTTP session */
  delay(4000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+HTTPACTION=0\\r\\n");
  SIM800L.println("AT+HTTPACTION=0");  /* Start GET session */
  delay(4000);
  ShowSerialData();
  delay(4000);
  Serial.print("AT+HTTPREAD\\r\\n");
  SIM800L.println("AT+HTTPREAD");  /* Read data from HTTP server */
  delay(4000);
  int i=0;char buff[64];
  while(SIM800L.available()!=0) // here you can analyze the result and put it into the return value - ret
    {
    buff[i]= (char)SIM800L.read(); i++;
    }
  Serial.printf("\nResult=%s",buff);  
  delay(4000);
  Serial.print("AT+HTTPTERM\\r\\n");  
  SIM800L.println("AT+HTTPTERM");  /* Terminate HTTP service */
  delay(4000);
  return ret;
 }
  

void loop() {
  Serial.println("HTTP get method :");
  Serial.print("AT\\r\\n");
  SIM800L.println("AT"); /* Check Communication */
  delay(5000);
  ShowSerialData(); /* Print response on the serial monitor */
  delay(5000);
  /* Configure bearer profile 1 */
  Serial.print("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\\r\\n");    
  SIM800L.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");  /* Connection type GPRS */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+SAPBR=3,1,\"APN\",\"free\"\\r\\n");  
  SIM800L.println("AT+SAPBR=3,1,\"APN\",\"free\"");  /* APN of the provider */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+SAPBR=1,1\\r\\n");
  SIM800L.println("AT+SAPBR=1,1"); /* Open GPRS context */
  delay(5000);
  ShowSerialData();
  delay(5000);
  Serial.print("AT+SAPBR=2,1\\r\\n");
  SIM800L.println("AT+SAPBR=2,1"); /* Query the GPRS context */
  delay(5000);
  ShowSerialData();
  delay(5000);

  // using active GPRS context with local IP
  for(int i=0;i<2;i++) req_HTTP("\"86.217.14.104:443/channels/174/fields/1/last?key=V7M6I771U8OJ2JSV\"");
  for(int i=0;i<2;i++) req_HTTP("\"86.217.14.104:443/channels/174/fields/2/last?key=V7M6I771U8OJ2JSV\"");

  delay(5000);
  Serial.print("AT+SAPBR=0,1\\r\\n");
  SIM800L.println("AT+SAPBR=0,1"); /* Close GPRS context */
  delay(5000);
  ShowSerialData();
  delay(5000);
}

void ShowSerialData()
{
  while(SIM800L.available()!=0)  /* If data is available on serial port */
  Serial.write(char (SIM800L.read())); /* Print character received on to the serial monitor */
}
