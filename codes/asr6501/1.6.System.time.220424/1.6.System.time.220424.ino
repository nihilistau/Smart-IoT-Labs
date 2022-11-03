TimerSysTime_t sysTimeCurrent;
void setup() {
  Serial.begin(9600);
  /*typedef struct TimerSysTime_s
  *{
  *  uint32_t Seconds;
  *  int16_t SubSeconds;
  *}TimerSysTime_t;
  */
  sysTimeCurrent = TimerGetSysTime( );
  Serial.printf("sys time:%u.%d\r\n",(unsigned int)sysTimeCurrent.Seconds, sysTimeCurrent.SubSeconds);
  TimerSysTime_t newSysTime ;
  newSysTime.Seconds = 1000;
  newSysTime.SubSeconds = 50;
  TimerSetSysTime( newSysTime );
  sysTimeCurrent = TimerGetSysTime( );
  Serial.printf("sys time:%u.%d\r\n",(unsigned int)sysTimeCurrent.Seconds, sysTimeCurrent.SubSeconds);
}

void loop() {

  delay(1000);
  sysTimeCurrent = TimerGetSysTime( );
  Serial.printf("sys time:%u.%d\r\n",(unsigned int)sysTimeCurrent.Seconds, sysTimeCurrent.SubSeconds);
}
