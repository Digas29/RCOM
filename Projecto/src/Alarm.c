#include "Alarm.h"

int timeExceeded = 0;

void alarmHandler(){
  timeExceeded = 1;
}
void setAlarm(int seconds){
  timeExceeded = 0;
  alarm(seconds);
}
void offAlarm(){
  timeExceeded = 0;
  alarm(0);
}
