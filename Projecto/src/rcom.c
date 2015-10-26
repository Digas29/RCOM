#include "alarm.h"
#include <signal.h>

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define DEFAULT_BAUDRATE B38400
#define FALSE 0
#define TRUE !FALSE
#define F 0x7E
#define A_SR 0x03
#define A_RS 0x01

int main(int argc, char** argv)
{
	(void) signal(SIGALRM, alarmHandler);

	return 0;
}
