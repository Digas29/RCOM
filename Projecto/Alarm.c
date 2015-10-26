#include "Alarm.h"
#include <unistd.h>

void alarmHandler(){
  timeExceeded = 1;
}
void setAlarm(int seconds){
  timeExceeded = 0;
  alarm(seconds);
}
void offAlarm(){
  alarm(0);
}
