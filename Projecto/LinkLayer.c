#include "Application.h"

int openPort(char * port){
  return open(porta, O_RDWR | O_NOCTTY );
}

int closePortAndResetTermios(){
  if ( tcsetattr(appLayer->fd,TCSANOW,&link->oldtio) == -1) {
  	perror("tcsetattr");
  	return 0;
  }
  close(appLayer->fd);
  return 1;
}

int startLinkLayer(char * port, Mode connectionMode, int baudrate, int dataMax, int timeout, int maxRetries){
  link = (LinkLay*)malloc(sizeof(LinkLayer));
  strcpy(link->port, port);
  link->mode = connectionMode;
  link->baudrate = baudrate;
  link->messageMaxSize = dataMax;
  link->timeout = timeout;
  link->numTransmissions = maxRetries;
  link->sequenceNumber = 0;
  if(!termiosSettings()){
    return 0;
  }
  return 1;
}

int termiosSettings(){
  if ( tcgetattr(appLayer->fd,&link->oldtio) == -1) { /* save current port settings */
    printf("Error in saving termios settings.\n ");
    return 0;
  }

  bzero(&link->newtio, sizeof(link->newtio));
  newtio.c_cflag = link->baudrate | CS8 | CLOCAL | CREAD;
  link->newtio.c_iflag = IGNPAR;
  link->newtio.c_oflag = OPOST;
  link->newtio.c_lflag = 0;

  link->newtio.c_cc[VTIME]    = 3;   /* inter-character timer unused */
  link->newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 chars received */

  if(tcflush(appLayer->fd, TCIFLUSH) != 0{
    printf("Error in flushing termios settings.\n ");
    return 0;
  }

  if ( tcsetattr(appLayer->fd,TCSANOW,&link->newtio) == -1) {
    printf("Error in setting termios settings.");
    return 0;
  }
  return 1;
}
int llopen(Mode connectionMode){

	return fd;
}
