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
  if(!startLinkLayer(appLayer->fd, port, connectionMode, baudRate, messageMaxSize, retries, timeout)){
    printf("Error in initialize Link Layer.\n");
    return 0;
  }
  if(appLayer->mode == TRANSMITTER){
    send();
  }
  else if(appLayer->mode == RECEIVER){
    receive();
  }
  closePortAndResetTermios(appLayer->fd);
  return 1;
}
int send(){
  FILE* file = fopen(appLayer->fileName, "rb");
  if(!file){
    printf("Error the specified file cannot be openned! \n");
    return 0;
  }

  fseek(file, 0, SEEK_END);
  unsigned int fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);
  char fileSizeString[20];
  sprintf(fileSizeString, "%u", fileSize);
  printf("File size: %u Bytes", fileSize);
  if(llopen(appLayer->fd,appLayer->mode) < 0){
    return 0;
  }
  if(sendControlPackage(START,appLayer->fileName, fileSizeString) < 0){
    printf("Error: can't send start control package\n");
    return 0;
  }
  //send file...

  if (fclose(file) != 0) {
    printf("Error: File was not closed....\n");
    return 0;
  }
  if(sendControlPackage(END,appLayer->fileName, fileSizeString) < 0){
    printf("Error: can't send end control package\n");
    return 0;
  }

  if(llclose(appLayer->fd,appLayer->mode) < 0){
    return 0;
  }
  printf("\n");
  printf("File transfered successfully. \n");
  return 1;
}

int receive(){
    if (llopen(appLayer->fd, appLayer->mode) <= 0)
		  return 0;

    int done = FALSE;
    while(!done){
      char package[APP_MAX_SIZE] = "";
      llread(appLayer->fd, package);
      if(package[0] == END){
        done = TRUE;
      }
      int i;
      for(i = 0; i < APP_MAX_SIZE; i++){
        if(package[i] == 0)
          break;
        printf("0x%x", package[i]);
      }
      memset(package, 0, APP_MAX_SIZE);
    }

	if (!llclose(appLayer->fd, appLayer->mode)) {
		printf("Erro Serial port was not closed.\n");
		return 0;
	}

	printf("\n");
	printf("File transfered successfully. \n");

	return 1;
}

int sendControlPackage(ControlPackage C, char* fileName, char * fileSize){
  unsigned int size = 5 + strlen(fileName) + strlen(fileSize);
  char CPackage[size];

  int i, j = 0;

  CPackage[j++] = C;
	CPackage[j++] = SIZE;
	CPackage[j++] = strlen(fileSize);
	for (i = 0; i < strlen(fileSize); i++)
		CPackage[j++] = fileSize[i];

	CPackage[j++] = NAME;
	CPackage[j++] = strlen(fileName);
	for (i = 0; i < strlen(fileName); i++)
		CPackage[j++] = fileName[i];

  return llwrite(appLayer->fd, CPackage, size);
}
