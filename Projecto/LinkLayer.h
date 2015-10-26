#pragma once

#define MAX_SIZE 256

#include "Connection.h"

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

extern LinkLayer* link;

int openPort(char * port);
int closePortAndResetTermios();

int startLinkLayer(char * port, Mode connectionMode, int baudrate, int dataMax, int time, int maxRetries);

int termiosSettings();

int llopen(Mode connectionMode);
int llwrite(int fd, char * buffer, int length);
int llread(int fd, char * buffer);
int llclose(int fd);
