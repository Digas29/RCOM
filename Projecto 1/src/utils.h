#pragma once

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE !FALSE
#define APP_MAX_SIZE 2000
#define MAX_SIZE 256

typedef struct{

int numberOfFrameReceived;
int numberOftimeOuts;
int numberOfREJ;


} Statistics;

Statistics* globalStatistics;

void initializeStatistics();

