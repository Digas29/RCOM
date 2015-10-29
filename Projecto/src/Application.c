#include "Application.h"

void progressBar(float current, float total) {
	float percentage = 100.0 * current / total;

	printf("\rCompleted: %6.2f%% [", percentage);

	int i, len = 51;
	int pos = percentage * len / 100.0;

	for (i = 0; i < len; i++)
		i <= pos ? printf("=") : printf(" ");

	printf("]");

	fflush(stdout);
}

int initAppLayer(char * port, Mode connectionMode, int baudRate, int messageMaxSize, int retries, int timeout, char* fileName){
  appLayer = (ApplicationLayer *) malloc(sizeof(ApplicationLayer));
  appLayer->fd = openPort(port);
	appLayer->messageMaxSize = messageMaxSize;
  if (appLayer->fd < 0) {
    printf("Serial port cannot be oppened!\n");
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
  printf("File size: %u Bytes \n", fileSize);
  if(llopen(appLayer->fd,appLayer->mode) < 0){
    return 0;
  }
  if(sendControlPackage(START,appLayer->fileName, fileSizeString) < 0){
    printf("Error: can't send start control package\n");
    return 0;
  }
	printf("Starting sending...\n");
	unsigned int bytes;
	char fBytes[appLayer->messageMaxSize + 4];
	int nPackages = 0;
	while((bytes = fread(&fBytes[4],sizeof(char), appLayer->messageMaxSize, file)) > 0){
		nPackages++;
		fBytes[0] = DATA;
		fBytes[1] = nPackages;
		fBytes[2] = bytes >> 8;
		fBytes[3] = bytes & 0xFF;
		if(llwrite(appLayer->fd, fBytes, bytes + 4) == -1){
			printf("Error cannot send package %d... \n", nPackages);
			return 0;
		}
		progressBar(nPackages, fileSize/appLayer->messageMaxSize+1);
	}
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
		FILE* file; 
		char fileName[20] = "";
		char fileSizeS[20] = "";
 		char package[APP_MAX_SIZE];
		memset(package, 0, APP_MAX_SIZE);

    while(!done){
     
      int size = llread(appLayer->fd, package);
			if(size == -1){
				continue;
}
			int nPackages = 0;
      if(package[0] == END){
        done = TRUE;
      }
			if(package[0] == START){
				int j = 1;
				while(j < size){
					int i = 0;
					if(package[j] == SIZE){
						for(; i < package[j+1]; i++){
							fileSizeS[i] = package[i+j+2];
						}
					}
					else if(package[j] == NAME){
						for(; i < package[j+1]; i++){
							fileName[i] = package[i+j+2];
						}
					}
					if(j == 1)
						printf("Starting Reading the Size from First Package...\n");
					else printf("Starting Reading the Name from First Package...\n");
					j+= i + 2;
				}
				file = fopen(fileName, "wb");
				if(!file){
					printf("Error the specified file cannot be created! \n");
					return 0;
				}
			}
			if(package[0] == DATA && file != NULL){

				printf("Data package: %d received!\n", package[1]);
				int tamanho = (unsigned char)package[2] << 8 | (unsigned char)package[3];
				fwrite(&package[4], sizeof(char), tamanho,file);
			}
			
    	memset(package, 0, APP_MAX_SIZE);
  }
  if (fclose(file) != 0) {
    printf("Error: %s was not closed....\n", fileName);
    return 0;
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
