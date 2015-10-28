#pragma once

#include "Connection.h"
#include "LinkLayer.h"
#include "utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>


typedef enum {
	DATA = 0,
	START = 1,
	END = 2
} ControlPackage;

typedef enum {
	SIZE = 0,
	NAME = 1
} Type;

typedef struct  {
	int fd; /*Descritor correspondente à porta série*/
	Mode mode; /*TRANSMITTER | RECEIVER*/
  char* fileName;
} ApplicationLayer;

ApplicationLayer * appLayer;

int initAppLayer(char * port, Mode connectionMode, int baudRate, int messageMaxSize, int retries, int timeout, char* fileName);

int send();
int receive();
int sendControlPackage(ControlPackage C, char* fileName, char* fileSize);
