#include "Alarm.h"
#include "Application.h"
#include <signal.h>

int main(int argc, char** argv)
{
	(void) signal(SIGALRM, alarmHandler);

	return 0;
}
