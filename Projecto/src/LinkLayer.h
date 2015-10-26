#pragma once

#define MAX_SIZE 256
#define F 0x7E
#define A_SR 0x03
#define A_RS 0x01
#define SupervisionSize 5

#include "Connection.h"
#include "Application.h"
#include "Alarm.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef enum {
	C_SET = 0x03, C_UA = 0x07, C_RR = 0x05, C_REJ = 0x01, C_DISC = 0x0B
} Control;

typedef struct {
	char port[20]; /*Dispositivo /dev/ttySx, x = 0, 1*/

	int baudRate; /*Velocidade de transmissão*/

	unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/

	unsigned int timeout; /*Valor do temporizador: 1 s*/

	unsigned int numTransmissions; /*Número de tentativas em caso de falha*/

  unsigned int messageMaxSize;

	char frame[MAX_SIZE]; /*Trama*/

  struct termios oldtio,newtio;

} LinkLayer;

extern LinkLayer* linkLayer;

int openPort(char * port);
int closePortAndResetTermios();

int startLinkLayer(char * port, Mode connectionMode, int baudrate, int dataMax, int time, int maxRetries);

int termiosSettings();

int llopen(Mode connectionMode);
int llwrite(int fd, char * buffer, int length);
int llread(int fd, char * buffer);
int llclose(int fd);
