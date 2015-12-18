#pragma once
#include <unistd.h>

extern int timeExceeded;

void alarmHandler();
void setAlarm(int seconds);
void offAlarm();
