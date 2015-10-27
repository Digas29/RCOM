#include "Alarm.h"
#include "Application.h"
#include <signal.h>

int main(int argc, char** argv)
{
	(void) signal(SIGALRM, alarmHandler);
	if(strcmp(argv[1], "1") == 0)
		initAppLayer("/dev/ttyS4", TRANSMITTER, 38400, 20000, 3, 3, "pinguim.gif");
	else
		initAppLayer("/dev/ttyS0", SENDER, 38400, 20000, 3, 3, "pinguim.gif");
	return 0;
}
