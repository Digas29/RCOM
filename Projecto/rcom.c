#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "rcom.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define DEFAULT_BAUDRATE B38400
#define FALSE 0
#define TRUE !FALSE
#define F 0x7E
#define A_SR 0x03
#define A_RS 0x01
#define C_SET 0x07
#define BCC (A^C_SET)

int main(int argc, char** argv)
{

	// if ( (argc < 2) ||
	// (strcmp("/dev/ttyS4", argv[1])!=0) ) {
	// 	printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
	// 	exit(1);
	// }

	return 0;
}
