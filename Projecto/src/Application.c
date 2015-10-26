#include "Application.h"
int initAppLayer(char * port, Mode connectionMode, int baudRate, int messageMaxSize, int retries, int timeout, char* fileName){
  appLayer = (ApplicationLayer *) malloc(sizeof(ApplicationLayer));
  appLayer->fd = openPort(port);
  if (appLayer->fd < 0) {
    printf("Serial port cannot be oppened!");
    return 0;
  }
  appLayer->mode = connectionMode;
  appLayer->fileName = fileName;
  if(!startLinkLayer(port,connectionMode, baudRate, messageMaxSize, retries, timeout)){
    printf("Error in initialize Link Layer.\n");
    return 0;
  }
  if(appLayer->mode == TRANSMITTER){
    send();
  }
  else if(appLayer->mode == RECEIVER){
    receive();
  }
  closePortAndResetTermios();
  return 1;
}
int send(){
  FILE* file = fopen(appLayer->fileName, "rb");
  if(!file){
    printf("Error the specified file cannot be openned! \n");
    return 0;
  }

  fseek(file, 0, SEEK_END);
  int fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  char fileSizeString[20];
  sprintf(fileSizeString, "%d", fileSize);

  int fd = llopen(appLayer->mode);
}

int receive(){

}
