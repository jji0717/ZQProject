
#include "AlarmClient.h"

int main(void)
{
	AlarmClient ac("Software\\ZQ\\ISAAlarm");
	if (!ac.startTrigger())
		printf("can not start trigger\n");

	system("pause");

	ac.stopTrigger();

	return 0;
}
