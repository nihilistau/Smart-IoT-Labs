const int buttonPin = 17;    // 17 for wemos shield, 0 for PRG
long ISMband;
uint8_t devID;

uint8_t setdevID()
{
int did=1;
int  buttonState;
u8x8.clear(); 
char dbuf[32];
u8x8.drawString(0, 0,"Setting Dev ID");
while(1)
  {  
  buttonState = digitalRead(buttonPin);
  if(buttonState) { did++; if(did==24) did=1;
                    Serial.println(did);
                    sprintf(dbuf,"devID=%2.2d",did);
                    u8x8.drawString(0, 2,dbuf);

                    
                  }
  else 
     { 
      Serial.println(did); 
      u8x8.drawString(0, 4,"Device ID");
      sprintf(dbuf,"set to %2.2d",did);
             u8x8.drawString(0, 5,dbuf);
      return did;
      }
  delay(3000);
  }
}


long setISMband()
{
int count=0;
int  buttonState;
char dbuf[32];
u8x8.clear(); 
u8x8.drawString(0, 0,"Set ISM band");
while(1)
  {  
  buttonState = digitalRead(buttonPin);
  if(buttonState) 
    { 
      if(!count)count=1;else count=0;
      Serial.println(count);
      if(count) u8x8.drawString(4, 3,"868 MHz");
      else u8x8.drawString(4, 3,"433 MHz");
    }
  else
    {
    if(count) u8x8.drawString(0, 3,"Set to 868 MHz");
    else u8x8.drawString(0, 3,"Set to 433 MHz");
    if(count) return 868E6;
    else return 433E6;
    }
  delay(3000);
  }
}

   
int setparam()
{
int count=0;
int  buttonState;
char dbuf[32];
u8x8.clear(); 
u8x8.drawString(0, 0,"Set Parameters");
while(1)
  {  
  buttonState = digitalRead(buttonPin);
  if(buttonState) 
    { 
      if(!count)count=1;else count=0;
      Serial.println(count);
      if(count) u8x8.drawString(0, 1,"YES");
      else u8x8.drawString(0, 1,"NO ");
  }
  else { Serial.println(count); 
        if(count) 
          {
            u8x8.drawString(0, 2,"Starting to set");
            u8x8.drawString(0, 4,"Spreading Factor");
            u8x8.drawString(0, 5,"Signal bandwidth");
            u8x8.drawString(0, 6,"Frequency");
          }
        else 
            {
            u8x8.drawString(0, 2,"Setting defaults");
            u8x8.drawString(0, 4,"SF=8");
            u8x8.drawString(0, 5,"SB=125 KHz");
            u8x8.drawString(0, 6,"freq=433 MHz");
            }
  return count;
  }
  delay(3000);
  }
}

int setsf()
{
int sf=5;
int  buttonState;
u8x8.clear(); 
char dbuf[32];
u8x8.drawString(0, 0,"Setting SF");
while(1)
  {  
  buttonState = digitalRead(buttonPin);
  if(buttonState) { sf++; if(sf==13) sf=6;
                    Serial.println(sf);
                    sprintf(dbuf,"SF=%2.2d",sf);
                    u8x8.drawString(0, 2,dbuf);

                    
                  }
  else 
     { 
      Serial.println(sf); 
      u8x8.drawString(0, 4,"Spreading Factor");
      sprintf(dbuf,"set to %2.2d",sf);
             u8x8.drawString(0, 5,dbuf);
      return sf;
      }
  delay(3000);
  }
}

int setsb()
{
const int inval=31250;
int sb=inval; int i=0;
int  buttonState;
u8x8.clear(); 
char dbuf[32];
u8x8.drawString(0, 0,"Setting SB");
while(1)
  {  
  buttonState = digitalRead(buttonPin);
  if(buttonState) 
  {
    sb=inval*pow(2,i);i++; 
    Serial.println(sb);
    sprintf(dbuf,"SB=%d Hz",sb);
    u8x8.drawString(0, 2,dbuf);
    if(sb==500000) i=0;
  }
  else { Serial.println(sb); 
           u8x8.drawString(0, 4,"Signal Band set");
           sprintf(dbuf,"to %d Hz",sb);
           u8x8.drawString(0, 5,dbuf);return sb;
           }
  delay(3000);
  }
}

long setfreq()
{
long inval=ISMband;
int freq=inval; int i=0;
int  buttonState;
u8x8.clear(); 
char dbuf[32];
u8x8.drawString(0, 0,"Setting Freq");
while(1)
  {  
  buttonState = digitalRead(buttonPin);
  if(buttonState) 
  {
    freq=inval+250E3*i;i++; 
    Serial.println(freq);
    sprintf(dbuf,"Freq=%d KHz",freq/1000);
    u8x8.drawString(0, 2,dbuf);
    if(freq==435E6) i=0;
  }
  else { Serial.println(freq); 
         u8x8.drawString(0, 4,"Frequency set");
         sprintf(dbuf,"to %d KHz",freq/1000);
         u8x8.drawString(0, 5,dbuf);
             return freq;}
  delay(3000);
  }
}



void introd(int *psf,int *psb, long *pfreq)
{
int sf,sb; long freq; 
int par=0; // default no configuration 
ISMband=setISMband(); 
delay(5000);
devID=setdevID();
delay(5000);
par=setparam();
delay(5000);
Serial.println(par);

if(par)
{
sf=setsf();
delay(5000);
Serial.println(sf);
sb=setsb();
Serial.println(sb);
delay(5000);
freq=setfreq();
Serial.println(freq);
delay(5000);
}
else
Serial.println("defaults");

if(par)
  {
//    Serial.print("SF:"); Serial.println(sf);
//    Serial.print("SB:"); Serial.println(sb);
//    Serial.print("Freq:"); Serial.println(freq);
  }
  else
  {
    sf=8;sb=125000;freq=ISMband;
//    Serial.print("SF:"); Serial.println(sf);
//    Serial.print("SB:"); Serial.println(sb);
//    Serial.print("Freq:"); Serial.println(freq);
  }
*psf= sf; *psb=sb; *pfreq=ISMband;
}

