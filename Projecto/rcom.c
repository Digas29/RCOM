#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "rcom.h"

volatile int STOP=FALSE;

int llopen(int porta, TRANSMITTER | RECEIVER){
	char porta[20];
	sprintf(porta, "/dev/ttyS%d", porta);
	int fd =  open(porta, O_RDWR | O_NOCTTY );
	if (fd < 0) {perror(argv[1]); exit(-1); }

	if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = OPOST;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */


	tcflush(fd, TCIFLUSH);

	if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}
	return fd;
}
int llwrite(int fd, char * buffer, int length){
	int i;
	for(i = 0; i < length; i++){
		if(write(fd, &buffer[i], 1) < -1)
		return -1;
	}
	return i;
}
int llread(int fd, char * buffer){
	return read(fd, buffer, 1);
}
int llclose(int fd){
	return close(fd);
}
int main(int argc, char** argv)
{
	struct termios oldtio,newtio;
	char buf[255];
	int fd, res;


	if ( (argc < 2) ||
	(strcmp("/dev/ttyS4", argv[1])!=0) ) {
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
		exit(1);
	}

	fd = llopen(4, TRANSMITTER);
	/*
	SET[0] = F;
	SET[1] = A;
	SET[2] = C_SET;
	SET[3] = BCC;
	SET[4] = F;

	send_SET();
	alarm(5);
	int i = 0;
	while(i < 5){
	res = read(fd,&UA[i],1);
	i++;
}
flag = TRUE;

printf("Recieve: 0x%x 0x%x 0x%x 0x%x 0x%x \n", UA[0], UA[1], UA[2], UA[3], UA[4]);
*/

sleep(5);

if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
	perror("tcsetattr");
	exit(-1);
}
close(fd);
return 0;
}
