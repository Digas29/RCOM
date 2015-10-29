#include "LinkLayer.h"

int getBaudrate(int baudrate) {
	switch (baudrate) {
	case 0:
		return B0;
	case 50:
		return B50;
	case 75:
		return B75;
	case 110:
		return B110;
	case 134:
		return B134;
	case 150:
		return B150;
	case 200:
		return B200;
	case 300:
		return B300;
	case 600:
		return B600;
	case 1200:
		return B1200;
	case 1800:
		return B1800;
	case 2400:
		return B2400;
	case 4800:
		return B4800;
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	case 230400:
		return B230400;
	case 460800:
		return B460800;
	default:
		return -1;
	}
}
int openPort(char * port){
  return open(port, O_RDWR | O_NOCTTY );
}

int closePortAndResetTermios(int fd){
  if ( tcsetattr(fd,TCSANOW,&linkLayer->oldtio) == -1) {
  	perror("tcsetattr");
  	return 0;
  }
  close(fd);
  return 1;
}

int startLinkLayer(int fd, char * port, Mode connectionMode, int baudrate, int dataMax, int timeout, int maxRetries){
  linkLayer = (LinkLayer*)malloc(sizeof(LinkLayer));
  strcpy(linkLayer->port, port);
  linkLayer->baudRate = baudrate;
  linkLayer->messageMaxSize = dataMax;
  linkLayer->timeout = timeout;
  linkLayer->numTransmissions = maxRetries;
  linkLayer->sequenceNumber = 0;
  if(!termiosSettings(fd)){
    return 0;
  }
  return 1;
}

int termiosSettings(int fd){
  if ( tcgetattr(fd,&linkLayer->oldtio) == -1) { /* save current port settings */
    printf("Error in saving termios settings.\n ");
    return 0;
  }

  bzero(&linkLayer->newtio, sizeof(linkLayer->newtio));
  linkLayer->newtio.c_cflag = getBaudrate(linkLayer->baudRate) | CS8 | CLOCAL | CREAD;
  linkLayer->newtio.c_iflag = IGNPAR;
  linkLayer->newtio.c_oflag = OPOST;
  linkLayer->newtio.c_lflag = 0;

  linkLayer->newtio.c_cc[VTIME]    = 3;   /* inter-character timer unused */
  linkLayer->newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 chars received */

  if(tcflush(fd, TCIFLUSH) != 0){
    printf("Error in flushing termios settings.\n ");
    return 0;
  }

  if ( tcsetattr(fd,TCSANOW,&linkLayer->newtio) == -1) {
    printf("Error in setting termios settings.");
    return 0;
  }
  return 1;
}

int llopen(int fd, Mode connectionMode){
  printf("Establishing a connection...\n");

	int tries = 0, connected = 0;

	if(connectionMode == TRANSMITTER) {
		while (!connected) {
			if (tries == 0 || timeExceeded) {
				if (tries >= linkLayer->numTransmissions) {
					printf("Error: number of tries exceeded.\n");
					printf("Connection aborted.\n");
					return -1;
				}

        char* SET = malloc(SupervisionSize * sizeof(char));

      	SET[0] = F;
      	SET[1] = A_SR;
      	SET[2] = C_SET;
      	SET[3] = SET[1] ^ SET[2];
      	SET[4] = F;
        sendSupervisonFrame(fd, SET);
        free(SET);
				tries++;
				setAlarm(linkLayer->timeout);
			}
      char response[MAX_SIZE];
      recieveSupervisonFrame(fd, response);
      if(response[1] == A_SR && response[2] == C_UA){
        offAlarm();
        printf("Connection established! :)\n");
        connected = 1;
      }
		}
	}
	else if(connectionMode == RECEIVER){
		while (!connected) {
      char SET[MAX_SIZE];
      recieveSupervisonFrame(fd, SET);
      if(SET[1] == A_SR && SET[2] == C_SET){
        char* UA = malloc(SupervisionSize * sizeof(char));

      	UA[0] = F;
      	UA[1] = A_SR;
      	UA[2] = C_UA;
      	UA[3] = UA[1] ^ UA[2];
      	UA[4] = F;

				sendSupervisonFrame(fd, UA);
        free(UA);
        connected = 1;
				printf("Connection established! :)\n");
      }
		}
	}

	return fd;
}
int llwrite(int fd, char * buffer, unsigned int length){
  char* frame = (char*)malloc(length + DataSize * sizeof(char));
  char BCC_2 = BCC2(buffer, length);
  frame[0] = F;
  frame[1] = A_SR;
  frame[2] = (linkLayer->sequenceNumber << 5);
  frame[3] = frame[1]^frame[2];
  memcpy(&frame[4], buffer, length);
  frame[4 + length] = BCC_2;
  frame[5 + length] = F;

  int newSize = stuff(frame, length + DataSize * sizeof(char));

  int done = FALSE;
  int tries = 0;
  while(!done){
    if(tries == 0 || timeExceeded){
      if (tries >= linkLayer->numTransmissions) {
        printf("Error: number of tries exceeded.\n");
        printf("Connection aborted.\n");
        return -1;
      }
      if(write(fd, frame, newSize) == -1){
        printf("Error in write data frame...\n");
        return -1;
      }
      tries++;
      setAlarm(linkLayer->timeout);
    }
    char response[MAX_SIZE];
    
		recieveSupervisonFrame(fd, response);
    if(response[1] == A_SR || response[0] == F){
      if((response[2] & 0x0F) == C_REJ){
	globalStatistics->numberOfREJ++;
        if((response[2] >> 5) == linkLayer->sequenceNumber){
          offAlarm();
          tries = 0;
        }
        else{
          printf("Something went really wrong in REJ... \n");
          return -1;
        }
      }
      else if ((response[2] & 0x0F) == C_RR){
        if((response[2] >> 5) != linkLayer->sequenceNumber){
          offAlarm();
          linkLayer->sequenceNumber = (response[2] >> 5);
          done = TRUE;
	globalStatistics->numberOfFrameReceived++;
        }
        else{
					timeExceeded = 1;
        }
      }
    }
  }
  return newSize;
}
int llread(int fd, char * data){
  volatile int done = FALSE;

  int estado = 0;
  int size = 0;

  char * buffer = (char*)malloc(APP_MAX_SIZE*2);
  while(!done){
    char c;
    if(estado < 3){
			int retorno = read(fd, &c, 1);
      if(retorno == -1){
        printf("Error in llread()!! \n");
        return -1;
      }
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
			if(c == F){
        buffer[size] = c;
        size++;
        estado++;
      }
			else{
        buffer[size] = c;
        size++;
			}
      break;
    default:
      done = TRUE;
      break;
    }
  }
  int process = FALSE;
  int newSize = destuff(buffer, size);
	if(buffer[0] != F || buffer[1] != A_SR || buffer[3] != (buffer[1] ^ buffer[2])){
		printf("Frame received with BCC wrong... \n");
		return -1;
	}

  int dataSize = newSize - DataSize * sizeof(char);

  char BCC = BCC2(&buffer[4], dataSize);
  unsigned int sn = (buffer[2] >> 5) & 1;
  char response[SupervisionSize * sizeof(char)];
  response[0] = F;
  response[1] = A_SR;
  response[4] = F;
  if(linkLayer->sequenceNumber == sn){
    if(BCC != buffer[newSize - 2]){ //if frame got corrupted
      printf("Frame received with BCC2 wrong... \n Rejecting frame (REJ)... \n");
      response[2] = (linkLayer->sequenceNumber << 5) | C_REJ;
    }
    else{
      if(linkLayer->sequenceNumber == 0){
        linkLayer->sequenceNumber = 1;
      }
      else{
        linkLayer->sequenceNumber = 0;
      }
      process = TRUE;
      response[2] = (linkLayer->sequenceNumber << 5) | C_RR;
    }
  }
  else{
    response[2] = (linkLayer->sequenceNumber << 5) | C_RR;
  }
  response[3] = response[1] ^ response[2];
  sendSupervisonFrame(fd, response);
  if(process){
    memcpy(data, &buffer[4], dataSize);
		free(buffer);
    return dataSize;
  }
	free(buffer);
  return -1;
}
int llclose(int fd, Mode connectionMode){
  printf("\nDisconnecting...\n");

	int tries = 0, disconnected = 0;

	if(connectionMode == TRANSMITTER) {
		while (!disconnected) {
			if (tries == 0 || timeExceeded) {
				if (tries >= linkLayer->numTransmissions) {
					printf("Error: number of tries exceeded.\n");
					printf("Disconnection aborted.\n");
					return -1;
				}

        char* DISC = malloc(SupervisionSize * sizeof(char));

      	DISC[0] = F;
      	DISC[1] = A_SR;
      	DISC[2] = C_DISC;
      	DISC[3] = DISC[1] ^ DISC[2];
      	DISC[4] = F;
        sendSupervisonFrame(fd, DISC);
        free(DISC);
				tries++;
				setAlarm(linkLayer->timeout);
			}

      char response[MAX_SIZE];
      recieveSupervisonFrame(fd, response);
      if(response[1] == A_RS && response[2] == C_DISC){
        disconnected = 1;
        offAlarm();
        char* UA = malloc(SupervisionSize * sizeof(char));

      	UA[0] = F;
      	UA[1] = A_RS;
      	UA[2] = C_UA;
      	UA[3] = UA[1] ^ UA[2];
      	UA[4] = F;

				sendSupervisonFrame(fd, UA);
        free(UA);
        printf("Disconnected! :)\n");
      }
		}
	}
  else if(connectionMode == RECEIVER){
    char DISC_S[MAX_SIZE];
    recieveSupervisonFrame(fd, DISC_S);
    if(DISC_S[1] == A_SR && DISC_S[2] == C_DISC){
      while (!disconnected) {

        if (tries == 0 || timeExceeded) {
          if (tries >= linkLayer->numTransmissions) {
  					printf("Error: number of tries exceeded.\n");
  					printf("Disconnection aborted.\n");
  					return -1;
  				}
          char* DISC = malloc(SupervisionSize * sizeof(char));

          DISC[0] = F;
          DISC[1] = A_RS;
          DISC[2] = C_DISC;
          DISC[3] = DISC[1] ^ DISC[2];
          DISC[4] = F;

          sendSupervisonFrame(fd, DISC);
          tries++;
          free(DISC);
          setAlarm(linkLayer->timeout);
        }
        char UA[MAX_SIZE];
        recieveSupervisonFrame(fd, UA);
        if(UA[1] == A_RS && UA[2] == C_UA){
          disconnected = 1;
          printf("Disconnected! :)\n");
        }
      }
    }
  }

	return 1;
}
int sendSupervisonFrame(int fd, char * frame){
  return write(fd, frame, SupervisionSize * sizeof(char));
}
int recieveSupervisonFrame(int fd, char * frame){
  volatile int done = FALSE;

  int estado = 0;
  int size = 0;

  while(!done){
    char c;
		if(estado < 5){
			int retorno = read(fd, &c, 1);
      if(retorno == -1){
        printf("Error in llread()!! \n");
        return -1;
      }
			else if (retorno == 0 && timeExceeded == 1){
				return -1;
			}
    }

    switch(estado){
    case 0:
      if(c == F){
        frame[size] = c;
        size++;
        estado++;
      }
      break;
    case 1:
        if(c == A_SR || c == A_RS){
          frame[size] = c;
          size++;
          estado++;
        }
        else if(c != F){
					printf("Receiving garbage\n");
          size = 0;
          estado = 0;
        }
        break;
    case 2:
      if(c == C_SET || c == C_UA || (c & 0x0F) == C_RR || (c & 0x0F) == C_REJ || c == C_DISC){
        frame[size] = c;
        size++;
        estado++;
      }
      else if (c == F){
        size = 1;
        estado = 1;
      }
      else{
				printf("Receiving garbage\n");
        size = 0;
        estado = 0;
      }
      break;
    case 3:
      if(c == (frame[1] ^ frame[2])){
        frame[size] = c;
        size++;
        estado++;
      }
      else if (c == F){
        size = 1;
        estado = 1;
      }
      else{
				printf("Receiving garbage\n");
        size = 0;
        estado = 0;
      }
      break;
    case 4:
        if(c == F){
          frame[size] = c;
          size++;
          estado++;
        }
        else{
					printf("Receiving garbage\n");
          size = 0;
          estado = 0;
        }
      break;
    default:
      done = TRUE;
      break;
    }
  }
	
  return size;
}

unsigned int stuff(char* buf, unsigned int frameSize){
	unsigned int newframeSize = frameSize;

	int i;
	for (i = 1; i < frameSize - 1; i++)
		if (buf[i] == F || buf[i] == ESCAPE)
			newframeSize++;
	buf = (char*) realloc(buf, newframeSize);
	for (i = 1; i < frameSize - 1; i++) {
		if (buf[i] == F || buf[i] == ESCAPE) {
			memmove(buf + i + 1, buf + i, frameSize - i);
			frameSize++;
			buf[i] = ESCAPE;
			buf[i + 1] ^= 0x20;
		}
	}

	return newframeSize;
}
unsigned int destuff(char* buf, unsigned int frameSize){
	int i;
	for (i = 1; i < frameSize - 1; ++i) {
		if (buf[i] == ESCAPE) {
			memmove(buf + i, buf + i + 1, frameSize - i - 1);

			frameSize--;

			buf[i] ^= 0x20;
		}
	}

	buf = (char*) realloc(buf, frameSize);

	return frameSize;
}

char BCC2(char* data, unsigned int dataSize){
  int i;
  char BCC = 0;
  for(i = 0; i < dataSize; i++){
    BCC ^= data[i];
  }
  return BCC;
}
