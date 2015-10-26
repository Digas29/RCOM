#pragma once

#include "Connection.h"
#include "LinkLayer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct  {
	int fd; /*Descritor correspondente à porta série*/
	Mode mode; /*TRANSMITTER | RECEIVER*/
  char* fileName;
} ApplicationLayer;

extern ApplicationLayer * appLayer;

int initAppLayer(char * port, Mode connectionMode, int baudRate, int messageMaxSize, int retries, int timeout, char* fileName);

int send();
int receive();
