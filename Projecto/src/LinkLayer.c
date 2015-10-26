#include "LinkLayer.h"

int openPort(char * port){
  return open(port, O_RDWR | O_NOCTTY );
}

int closePortAndResetTermios(){
  if ( tcsetattr(appLayer->fd,TCSANOW,&linkLayer->oldtio) == -1) {
  	perror("tcsetattr");
  	return 0;
  }
  close(appLayer->fd);
  return 1;
}

int startLinkLayer(char * port, Mode connectionMode, int baudrate, int dataMax, int timeout, int maxRetries){
  linkLayer = (LinkLayer*)malloc(sizeof(LinkLayer));
  strcpy(link->port, port);
  linkLayer->baudRate = baudrate;
  linkLayer->messageMaxSize = dataMax;
  linkLayer->timeout = timeout;
  linkLayer->numTransmissions = maxRetries;
  linkLayer->sequenceNumber = 0;
  if(!termiosSettings()){
    return 0;
  }
  return 1;
}

int termiosSettings(){
  if ( tcgetattr(appLayer->fd,&linkLayer->oldtio) == -1) { /* save current port settings */
    printf("Error in saving termios settings.\n ");
    return 0;
  }

  bzero(&linkLayer->newtio, sizeof(linkLayer->newtio));
  newtio.c_cflag = linkLayer->baudRate | CS8 | CLOCAL | CREAD;
  linkLayer->newtio.c_iflag = IGNPAR;
  linkLayer->newtio.c_oflag = OPOST;
  linkLayer->newtio.c_lflag = 0;

  linkLayer->newtio.c_cc[VTIME]    = 3;   /* inter-character timer unused */
  linkLayer->newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 chars received */

  if(tcflush(appLayer->fd, TCIFLUSH) != 0){
    printf("Error in flushing termios settings.\n ");
    return 0;
  }

  if ( tcsetattr(appLayer->fd,TCSANOW,&linkLayer->newtio) == -1) {
    printf("Error in setting termios settings.");
    return 0;
  }
  return 1;
}

int llopen(Mode connectionMode){
  printf("Establishing a connection...\n");

	int tries = 0, connected = 0;

	if(connectionMode == TRANSMITTER) {
		while (!connected) {
			if (tries == 0 || timeExceeded) {
				if (tries >= linkLayer->numTransmissions) {
					printf("Error: number of tries exceeded.\n");
					printf("Connection aborted.\n");
					return 0;
				}

        char* SET = malloc(SupervisionSize * sizeof(char));

      	SET[0] = F;
      	SET[1] = A_SR;
      	SET[2] = C_SET;
      	SET[3] = SET[1] ^ SET[2];
      	SET[4] = F;

        write(appLayer->fd, SET, SupervisionSize * sizeof(char));
        free(SET);
				tries++;
				setAlarm(linkLayer->timeout);
			}

      char response[MAX_SIZE];
      llread(appLayer->fd, response);
      if(response[1] == A_SR && response[2] == C_UA){
        connected = 1;
        offAlarm();
      }
		}
	}
	else if(connectionMode == RECEIVER){
		while (!connected) {
      char SET[MAX_SIZE];
      llread(appLayer->fd, SET);
      if(SET[1] == A_SR && SET[2] == C_SET){
        char* UA = malloc(SupervisionSize * sizeof(char));

      	UA[0] = F;
      	UA[1] = A_SR;
      	UA[2] = C_SET;
      	UA[3] = UA[1] ^ UA[2];
      	UA[4] = F;

				write(appLayer->fd, UA, SupervisionSize * sizeof(char));
        free(UA);
        connected = 1;
				printf("Connection established! :)\n");
      }
		}
	}

	return appLayer->fd;
}

int llwrite(int fd, char * buffer, int length){
  return write(fd, buffer, length);
}

int llread(int fd, char * buffer){
  volatile int done = FALSE;

  int estado = 0;
  int size = 0;
  int tries = 0;

  while(!done){
    char c;
    if(read(fd, &c, 1) == -1){
      printf("Error in llread()!! \n");
      return -1;
    }

    switch(estado){
    case 0:
      if(c == F){
        buffer[size] = c;
        size++;
        estado++;
      }
      break;
    case 1:
        if(c == A_SR || c == A_RS){
          buffer[size] = c;
          size++;
          estado++;
        }
        else if(c != F){
          size = 0;
          estado = 0;
        }
        break;
    case 2:
      if(c == C_SET || c == C_UA || c == C_RR || c == C_REJ || c == C_DISC){
        buffer[size] = c;
        size++;
        estado++;
      }
      else if (c == F){
        size = 1;
        estado = 1;
      }
      else{
        size = 0;
        estado = 0;
      }
      break;
    case 3:
      if(c == (buffer[1] ^ buffer[2])){
        buffer[size] = c;
        size++;
        estado++;
      }
      else if (c == F){
        size = 1;
        estado = 1;
      }
      else{
        size = 0;
        estado = 0;
      }
    case 4:
      if(buffer[2] != 0 && buffer[2] != (1<<5)){
        if(c == F){
          buffer[size] = c;
          size++;
          return size;
        }
        else{
          size = 0;
          estado = 0;
        }
      }
      else{
        if(c == 0){
          estado++;
        }
        else{
          buffer[size] = c;
          size++;
        }
      }
      break;
    default:
      done = TRUE;
      break;
    }
  }
  //destuff
  return size;
}
