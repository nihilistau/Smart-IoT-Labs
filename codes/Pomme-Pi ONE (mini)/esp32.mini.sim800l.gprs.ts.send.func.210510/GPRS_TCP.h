HardwareSerial SIM900(2);

void ShowSerialData()
{
union 
  {
  uint8_t buff[128];
  char rec[128];    
  } rep;
int i=0;
while(SIM900.available()!=0) 
 {
  rep.buff[i]=SIM900.read(); i++;
 }
  Serial.println(rep.rec); /* Print character received on to the serial monitor */ 
}

int startgpsr_TCP(char *atn)
{
  Serial.println("AT\\r\\n");
  SIM900.println("AT"); /* Check Communication */
  delay(2000);
  ShowSerialData();  /* Print response on the serial monitor */
  delay(2000);
  Serial.println("AT+CIPMODE=0\\r\\n"); 
  SIM900.println("AT+CIPMODE=0"); /* Non-Transparent (normal) mode for TCP/IP application */
  delay(5000);
  ShowSerialData();
  delay(2000);
  Serial.println("AT+CIPMUX=0\\r\\n");
  SIM900.println("AT+CIPMUX=0");  /* Single TCP/IP connection mode */
  delay(5000);
  ShowSerialData();
  delay(2000);
  Serial.println("AT+CGATT=1\\r\\n");
  SIM900.println("AT+CGATT=1"); /* Attach to GPRS Service */
  delay(5000);
  ShowSerialData();
  delay(2000);
  Serial.println("AT+CREG?\\r\\n");
  SIM900.println("AT+CREG?"); /* Network registration status */
  delay(2000);
  ShowSerialData();
  delay(2000);
  Serial.println("AT+CGATT?\\r\\n");
  SIM900.println("AT+CGATT?");  /* Attached to or detached from GPRS service */ 
  delay(5000); 
  ShowSerialData();
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
  delay(2000);
  ShowSerialData();
  delay(2000);
return 0;
}


int sendgprs_TCP(char *request)
{
  char req[128], preq[128]; char buff[128];
  memset(buff,0x00,128);
  memset(req,0x00,128);memset(preq,0x00,128);

  strcpy(req,"GET /update?key=");
  strcat(req,request);
  strcat(req,"\r\n\x1A");
  Serial.println(req);
  Serial.println("AT+CIPSTART=\"TCP\",\"86.217.14.104\",\"443\"\\r\\n");
  SIM900.println("AT+CIPSTART=\"TCP\",\"86.217.14.104\",\"443\"");  /* Start up TCP connection */
  delay(2000);
  ShowSerialData();
  delay(2000);
  Serial.println("AT+CIPSEND\\r\\n");
  SIM900.println("AT+CIPSEND"); /* Send data through TCP connection */
  delay(2000);
  ShowSerialData();
  delay(2000);
//  Serial.print("GET /update?key=4M9QG56R7VGG8ONT&field1=0\\r\\n");
//  SIM900.print("GET /update?key=4M9QG56R7VGG8ONT&field1=0\r\n\x1A");   
  Serial.print(req);
  SIM900.print(req);   
  ShowSerialData();
  delay(2000);
  Serial.println();
  Serial.print("AT+CIPSHUT\\r\\n");
  SIM900.println("AT+CIPSHUT"); /* Deactivate GPRS PDP content */
  delay(2000);
  ShowSerialData();
  delay(4000);
  return 0;
}





