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



RTC_DATA_ATTR  uint32_t termID,gwID;
int stage1_flag=0;  // reception flag for control packets: GW-IDREQ, TERM-IDACK
int stage2_flag=0;  // reception flag for data packets: TS-GW-DTSND, TS-TERM-DTACK; TS-GW-DTREQ,TS-TERM-DTRCV 
// reception flag for data packets: MQTT-GW-DTPUB, MQTT-TERM-DTSUBACK; MQTT-GW-DTPUB, MQTT-TERM-DTSUBRCV
                    // TS

void onReceive(int packetSize) 
{
if(packetSize==32)
  { 
#ifdef TS  
  conframe_t rcf; int i=0;
  while (LoRa.available()) { rcf.frame[i] = LoRa.read();i++;}
    if(MODE==1 && rcf.pack.con[0]==0x11 && rcf.pack.did==0x00 )  // received IDREQ_TSS packet
    { stage1_flag=1; termID=rcf.pack.sid; Serial.println("Received IDREQ_TSS");  }  
    if(MODE==2 && rcf.pack.con[0]==0x21 && rcf.pack.did==0x00 )  // received IDREQ_TSR packet
    {  stage1_flag=1; termID=rcf.pack.sid; Serial.println("Received IDREQ_TSR");  }
    
    if(MODE==1 && rcf.pack.con[0]==0x12 && rcf.pack.did==termID )  // received IDACK_TSS packet
    { stage1_flag=1; gwID=rcf.pack.sid; Serial.println("Received IDACK_TSS");  }  
    if(MODE==2 && rcf.pack.con[0]==0x22 && rcf.pack.did==termID )  // received IDACK_TSR packet
    {  stage1_flag=1; gwID=rcf.pack.sid; Serial.println("Received IDACK_TSR");  }    
 #endif   
 #ifdef MQTT    
    if(MODE==3 && rcf.pack.con[0]==0x31 && rcf.pack.did==0x00 )  // received IDREQ_MQP packet
    { stage1_flag=1; termID=rcf.pack.sid; Serial.println("Received IDREQ_MQP"); }
    if(MODE==4 && rcf.pack.con[0]==0x41 && rcf.pack.did==0x00 )  // received IDREQ_MQS packet
    { stage1_flag=1; termID=rcf.pack.sid; Serial.println("Received IDREQ_MQS");  }  
    
    if(MODE==3 && rcf.pack.con[0]==0x32 && rcf.pack.did==termID )  // received IDACK_MQP packet
    { stage1_flag=1; gwID=rcf.pack.sid; Serial.println("Received IDACK_MQP"); }
    if(MODE==4 && rcf.pack.con[0]==0x42 && rcf.pack.did==termID )  // received IDACK_MQS packet
    { stage1_flag=1; gwID=rcf.pack.sid; Serial.println("Received IDACK_MQS");  }
  #endif    
  }
  
if(packetSize==64)
  { 
#ifdef TS 
  TSframe_t rdf; int i=0;
#endif 
#ifdef MQTT 
  MQTTframe_t rdf; int i=0;
#endif
  while (LoRa.available()) { rdf.frame[i] = LoRa.read();i++;}
    if(MODE==1 && rdf.pack.con[0]==0x13 )  // GW:received DTSND packet
    { stage2_flag=1; termID=rdf.pack.sid; Serial.println("Received DTSND");  }  
    if(MODE==2 && rdf.pack.con[0]==0x23 )  // GW:received DTREQ packet
    { stage2_flag=1; termID=rdf.pack.sid; Serial.println("Received DTREQ");  }   
    
    if(MODE==1 && rdf.pack.con[0]==0x14 )  // TERM:received DTACT packet
    { stage2_flag=1; gwID=rdf.pack.sid; Serial.println("Received DTACT");  }  
    if(MODE==2 && rdf.pack.con[0]==0x24 )  // TERM:received DTRCV packet
    { stage2_flag=1; gwID=rdf.pack.sid; Serial.println("Received IDTRCV");  }    

    if(MODE==3 && rdf.pack.con[0]==0x33 )  // GW:received DTPUB packet
    { stage2_flag=1; termID=rdf.pack.sid; Serial.println("Received DTSND");  }  
    if(MODE==4 && rdf.pack.con[0]==0x43 )  // TERM:received DTPUBACK packet
    { stage2_flag=1; gwID=rdf.pack.sid; Serial.println("Received DTREQ");  }   
    if(MODE==3 && rdf.pack.con[0]==0x34 )  // GW:received DTSUB packet
    { stage2_flag=1; termID=rdf.pack.sid; Serial.println("Received DTACT");  }  
    if(MODE==4 && rdf.pack.con[0]==0x44 )  // received DTSUBRCV packet
    { stage2_flag=1; gwID=rdf.pack.sid; Serial.println("Received DTSUBRCV");  } 
   }
  }
  
  
  
