#include "Alarm.h"
#include "Application.h"
#include <signal.h>

int main(int argc, char** argv)
{
	if(argc != 8)
	{
	printf("Number of Arguments Wrong: string port, int connectionMode, int baudRate, int messageMaxSize, int retries, int timeout, string fileName\n");
	return -1;
	}
	initializeStatistics();
	
	char* port = argv[1];
	int baudRate = atoi(argv[3]);
	int messageMaxSize = atoi(argv[4]);
	int retries = atoi(argv[5]);
	int timeout = atoi(argv[6]);
	char* fileName = argv[7];
	Mode connectionMode;

	(void) signal(SIGALRM, alarmHandler);

	
	if(strcmp(argv[2], "1") == 0)
		connectionMode = TRANSMITTER;
	else
		connectionMode = RECEIVER;

	initAppLayer(port, connectionMode, baudRate, messageMaxSize, retries, timeout, fileName);

	if(connectionMode != RECEIVER){
	FILE* fileptr = fopen("Statistics.txt","w");
	
	fprintf(fileptr,("Number of Frames Received/Transmited : %d\n"),globalStatistics->numberOfFrameReceived);
	fprintf(fileptr,("Number of Timeouts Occurrences : %d\n"),globalStatistics->numberOftimeOuts);
	fprintf(fileptr,("Number of REJ Received/Transmited : %d\n"),globalStatistics->numberOfREJ);
	
	free(globalStatistics);// free the global Statistics variable
	}
	return 0;
}

void initializeStatistics(){
	globalStatistics = (Statistics*)malloc(sizeof(Statistics));
	globalStatistics->numberOfFrameReceived = 0;
	globalStatistics->numberOftimeOuts = 0;
	globalStatistics->numberOfREJ = 0;
}

/*		initAppLayer("/dev/ttyS4", TRANSMITTER, 38400, 20000, 3, 3, "pinguim.gif");
	else
		initAppLayer("/dev/ttyS0", RECEIVER, 38400, 20000, 3, 3, "pinguim.gif");
*/

