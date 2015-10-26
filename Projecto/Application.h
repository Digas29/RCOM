#pragma once

#include "Connection.h"

typedef struct  {
	int fd; /*Descritor correspondente à porta série*/
	Mode mode; /*TRANSMITTER | RECEIVER*/
  char* fileName;
} ApplicationLayer;

extern ApplicationLayer * appLayer;

int initAppLayer(char * port, Mode connectionMode, int baudRate, int messageMaxSize, int retries, int timeout, char* fileName);

int send();
int receive();
